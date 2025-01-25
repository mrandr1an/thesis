#![no_std]
#![no_main]

use core::net::Ipv4Addr;

use embassy_executor::Spawner;
use embassy_net::{tcp::TcpSocket, Runner, StackResources};
use embassy_time::{Duration, Timer as TaskTimer};
use esp_backtrace as _;
use esp_hal::{
    clock::CpuClock,
    rng::Rng,
    timer::{
        systimer::SystemTimer,
        timg::{Timer, TimerGroup},
    },
};
use esp_wifi::{
    wifi::{
        ClientConfiguration, Configuration, WifiController, WifiDevice, WifiEvent, WifiStaDevice,
        WifiState,
    },
    EspWifiController,
};
use log::info;
use rust_iot_app::HCSR04;
use static_cell::StaticCell;

static WIFI_CONTROLLER_CELL: StaticCell<EspWifiController<'static>> = StaticCell::new();
static NW_STACK_RESOURCES: StaticCell<StackResources<3>> = StaticCell::new();
const SSID: &str = "Andrian";
const PASSWORD: &str = "12345678";

#[embassy_executor::task]
async fn sense_distance(mut hcsr04: HCSR04<'static, Timer>) {
    loop {
        let distance = hcsr04.measure_distance();
        info!("Distance measured: {}cm.", distance);
        TaskTimer::after_secs(1).await;
    }
}

#[esp_hal_embassy::main]
async fn main(spawner: Spawner) {
    let config = esp_hal::Config::default().with_cpu_clock(CpuClock::max());
    let peripherals = esp_hal::init(config);

    esp_alloc::heap_allocator!(72 * 1024);
    //Enable logging according to ESP_LOG Environment Variable
    esp_println::logger::init_logger_from_env();

    /* Peripheral Set up */

    /* Timers */

    let systimer = SystemTimer::new(peripherals.SYSTIMER);

    let timg0 = TimerGroup::new(peripherals.TIMG0);

    let timg1 = TimerGroup::new(peripherals.TIMG1);

    /* RNG (Random Number Generator) */

    let mut rng = Rng::new(peripherals.RNG);

    let wifi_controller = &*WIFI_CONTROLLER_CELL
        .init_with(|| esp_wifi::init(timg0.timer0, rng, peripherals.RADIO_CLK).unwrap());

    let (wifi_if, controller) =
        esp_wifi::wifi::new_with_mode(wifi_controller, peripherals.WIFI, WifiStaDevice).unwrap();

    //Embassy initilization
    esp_hal_embassy::init(systimer.alarm0);
    let embasy_net_config = embassy_net::Config::dhcpv4(Default::default());
    let seed = (rng.random() as u64) << 32 | rng.random() as u64;

    let stack = NW_STACK_RESOURCES.init_with(StackResources::new);

    let (stack, runner) = embassy_net::new(wifi_if, embasy_net_config, stack, seed);

    info!("Embassy initialized!");

    let hcsr04 = HCSR04::new(peripherals.GPIO3, peripherals.GPIO2, timg1.timer0);

    spawner.spawn(sense_distance(hcsr04)).unwrap();
    spawner.spawn(connection(controller)).ok();
    spawner.spawn(net_task(runner)).ok();
    let mut rx_buffer = [0; 4096];
    let mut tx_buffer = [0; 4096];

    loop {
        if stack.is_link_up() {
            break;
        }
        TaskTimer::after(Duration::from_millis(500)).await;
    }

    info!("Waiting to get IP address...");
    loop {
        if let Some(config) = stack.config_v4() {
            info!("Got IP: {}", config.address);
            break;
        }
        TaskTimer::after(Duration::from_millis(500)).await;
    }

    loop {
        TaskTimer::after(Duration::from_millis(1_000)).await;

        let mut socket = TcpSocket::new(stack, &mut rx_buffer, &mut tx_buffer);

        socket.set_timeout(Some(embassy_time::Duration::from_secs(10)));

        let remote_endpoint = (Ipv4Addr::new(142, 250, 185, 115), 80);
        info!("connecting...");
        let r = socket.connect(remote_endpoint).await;
        if let Err(e) = r {
            info!("connect error: {:?}", e);
            continue;
        }
        info!("connected!");
        let mut buf = [0; 1024];
        loop {
            use embedded_io_async::Write;
            let r = socket
                .write_all(b"GET / HTTP/1.0\r\nHost: www.mobile-j.de\r\n\r\n")
                .await;
            if let Err(e) = r {
                info!("write error: {:?}", e);
                break;
            }
            let n = match socket.read(&mut buf).await {
                Ok(0) => {
                    info!("read EOF");
                    break;
                }
                Ok(n) => n,
                Err(e) => {
                    info!("read error: {:?}", e);
                    break;
                }
            };
            info!("{}", core::str::from_utf8(&buf[..n]).unwrap());
        }
        TaskTimer::after(Duration::from_millis(3000)).await;
    }
}

#[embassy_executor::task]
async fn net_task(mut runner: Runner<'static, WifiDevice<'static, WifiStaDevice>>) {
    runner.run().await
}

#[embassy_executor::task]
async fn connection(mut controller: WifiController<'static>) {
    info!("start connection task");
    info!("Device capabilities: {:?}", controller.capabilities());
    loop {
        if esp_wifi::wifi::wifi_state() == WifiState::StaConnected {
            // wait until we're no longer connected
            controller.wait_for_event(WifiEvent::StaDisconnected).await;
            TaskTimer::after(Duration::from_millis(5000)).await
        }
        if !matches!(controller.is_started(), Ok(true)) {
            let client_config = Configuration::Client(ClientConfiguration {
                ssid: SSID.try_into().unwrap(),
                password: PASSWORD.try_into().unwrap(),
                ..Default::default()
            });
            controller.set_configuration(&client_config).unwrap();
            info!("Starting wifi");
            controller.start_async().await.unwrap();
            info!("Wifi started!");
        }
        info!("About to connect...");

        match controller.connect_async().await {
            Ok(_) => info!("Wifi connected!"),
            Err(e) => {
                info!("Failed to connect to wifi: {e:?}");
                TaskTimer::after(Duration::from_millis(5000)).await
            }
        }
    }
}
