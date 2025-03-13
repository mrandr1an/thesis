// Application Attributes
#![no_std]
#![no_main]

use core::str::FromStr;

use embassy_executor::{raw::Executor, task};
use embassy_net::{IpAddress, Runner, Stack, StackResources};
use embassy_sync::{blocking_mutex::raw::CriticalSectionRawMutex, signal::Signal};
use embassy_time::{Instant, Timer as TaskTimer};
use esp_hal::{
    clock::CpuClock,
    main,
    rng::Rng,
    timer::{
        systimer::{Alarm, SystemTimer},
        timg::TimerGroup,
    },
};
use esp_wifi::{
    wifi::{
        AuthMethod, ClientConfiguration, Configuration, WifiController, WifiDevice, WifiStaDevice,
        WifiState,
    },
    EspWifiController,
};
use heapless::String;
use log::info;
use static_cell::StaticCell;
use your_project::{
    hcsr04::HCSR04,
    mqttdriver::{Message, MqttDriver},
};

//Globals
static EXECUTOR: StaticCell<Executor> = StaticCell::new();
static WIFI_CONTROLLER: StaticCell<EspWifiController> = StaticCell::new();
static STA_STACK_RESOURCES: StaticCell<StackResources<3>> = StaticCell::new();
static RUN_MQTT: Signal<CriticalSectionRawMutex, bool> = Signal::new();
static DISTANCE_SIGNAL: Signal<CriticalSectionRawMutex, f64> = Signal::new();
static CPU_SIGNAL: Signal<CriticalSectionRawMutex, f64> = Signal::new();

//WiFi Credentials
//"VODAFONE_H268Q-5477"
//"H9bfKcZTYcDtXyyh"
const SSID: &str = "VODAFONE_H268Q-5477";
const PWD: &str = "H9bfKcZTYcDtXyyh";

extern crate alloc;

#[main]
fn main() -> ! {
    //Initiliazation Variables
    let config = esp_hal::Config::default().with_cpu_clock(CpuClock::max());
    let peripherals = esp_hal::init(config);
    esp_println::logger::init_logger_from_env();
    esp_alloc::heap_allocator!(72 * 1024);

    //Driver Peripheral Variables

    //Embassy Peripherals
    let timg1 = TimerGroup::new(peripherals.TIMG1);
    let timg0 = TimerGroup::new(peripherals.TIMG0);
    let mut rng = Rng::new(peripherals.RNG);
    let wifi = peripherals.WIFI;

    //HCSR04 Peripherals
    let timsys = SystemTimer::new(peripherals.SYSTIMER);
    let trigger = peripherals.GPIO11;
    let echo = peripherals.GPIO10;

    //Driver Initilization

    //WiFi Initialization
    let wifi_controller =
        &*WIFI_CONTROLLER.init(esp_wifi::init(timg0.timer0, rng, peripherals.RADIO_CLK).unwrap());

    let sta_resources = STA_STACK_RESOURCES.init(StackResources::new());
    let (wifi_sta, mut controller) =
        esp_wifi::wifi::new_with_mode(wifi_controller, wifi, WifiStaDevice).unwrap();

    let sta_config = embassy_net::Config::dhcpv4(Default::default());
    let rng_seed = (rng.random() as u64) << 32 | rng.random() as u64;
    let conf = Configuration::Client(ClientConfiguration {
        ssid: String::<32>::from_str(SSID).unwrap(),
        password: String::<64>::from_str(PWD).unwrap(),
        auth_method: AuthMethod::WPA2Personal,
        ..Default::default()
    });
    controller.set_configuration(&conf).unwrap();
    let (sta_stack, sta_runner) = embassy_net::new(wifi_sta, sta_config, sta_resources, rng_seed);
    //Embassy Initialization
    esp_hal_embassy::init(timg1.timer0);
    //HCSR04
    let hcsr04 = HCSR04::new(trigger, echo, timsys.alarm0);
    //MQTT Driver

    //Executor
    let executor = EXECUTOR.init(Executor::new(3 as *mut ()));
    let spawner = executor.spawner();
    spawner.must_spawn(sta_connection(sta_runner));
    spawner.must_spawn(wifi_connection(controller));
    spawner.must_spawn(mqtt(sta_stack));
    spawner.must_spawn(measure_distance(hcsr04));
    //Spawn tasks
    let mut acc = 0.0;
    let start_time = Instant::now();
    loop {
        let start_pol = Instant::now();
        unsafe { executor.poll() };
        let end_pol = Instant::now();
        let elapsed_pol = end_pol.duration_since(start_pol).as_ticks() as f64;
        acc += elapsed_pol;
        let total_elapsed = start_time.elapsed().as_ticks() as f64;
        // println!("CPU Usage: {}%", (acc * 100_f64 / total_elapsed));
        CPU_SIGNAL.signal(acc * 100_f64 / total_elapsed);
    }
}

#[task]
async fn measure_distance(mut sensor: HCSR04<'static, Alarm>) {
    loop {
        DISTANCE_SIGNAL.signal(sensor.measure_distance());
        TaskTimer::after_millis(250).await;
    }
}

#[task]
async fn wifi_connection(mut controller: WifiController<'static>) {
    info!("start connection task");
    info!("Device capabilities: {:?}", controller.capabilities());

    info!("Starting wifi");
    controller.start_async().await.unwrap();
    match controller.connect_async().await {
        Ok(_) => (),
        Err(err) => info!("Error connecting {:#?}", err),
    }
    info!("Wifi started!");

    loop {
        match esp_wifi::wifi::sta_state() {
            WifiState::StaConnected => {
                info!("Station has connected");
                //Signal status
                RUN_MQTT.signal(true);
                break;
            }
            e => info!("Other {:#?}", e),
        }
        TaskTimer::after_millis(500).await;
    }
}

#[task]
async fn sta_connection(mut runner: Runner<'static, WifiDevice<'static, WifiStaDevice>>) {
    runner.run().await;
}

#[task]
async fn mqtt(sta_stack: Stack<'static>) {
    //Wait for signal that WiFi_Sta has connected to a network
    let _run = RUN_MQTT.wait().await;

    //Wait to get IP.
    loop {
        if let Some(config) = sta_stack.config_v4() {
            info!("Got IP: {}", config.address);
            break;
        }
        TaskTimer::after_millis(500).await;
    }
    //TCP Connect to Broker

    let mqtt_driver = MqttDriver::new()
        .with_broker_ip(IpAddress::v4(77, 237, 242, 215), 1883)
        .with_id("esp32c6Client")
        .with_password("chris")
        .with_username("chris")
        .with_packet_size(1024)
        .with_keep_alive(300)
        .build(sta_stack)
        .await;

    match mqtt_driver {
        Ok(mut mqtt) => {
            //MQTT connect to broker
            let status = mqtt.connect().await;
            match status {
                Ok(_) => loop {
                    let current_distance = DISTANCE_SIGNAL.wait().await;
                    let cpu_usage = CPU_SIGNAL.wait().await;
                    let msg = Message::new(current_distance, cpu_usage);

                    if let Err(err) = mqtt.publish(msg).await {
                        info!("Received reason code {:#?}", err);
                    }
                },
                Err(err) => info!("Mqtt Error: {:#?}", err),
            }
        }
        Err(err) => info!("Tcp Error: {:#?}", err),
    }
}

#[panic_handler]
fn panic(_info: &core::panic::PanicInfo) -> ! {
    loop {}
}
