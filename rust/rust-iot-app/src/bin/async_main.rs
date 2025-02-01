#![no_std]
#![no_main]
use core::{any::Any, net::Ipv4Addr};
use embassy_executor::Spawner;
use embassy_net::{Ipv4Cidr, Runner, Stack, StackResources, StaticConfigV4};
use embassy_time::{Duration, Timer as TaskTimer};
use esp_backtrace as _;
use esp_hal::{
    clock::CpuClock,
    rng::Rng,
    timer::{systimer::SystemTimer, timg::TimerGroup},
};
use esp_hal_dhcp_server::{simple_leaser::SimpleDhcpLeaser, structs::DhcpServerConfig};
use esp_wifi::{
    wifi::{
        AccessPointConfiguration, ClientConfiguration, Configuration, WifiApDevice, WifiController,
        WifiDevice, WifiEvent, WifiState,
    },
    EspWifiController,
};
use log::info;
use static_cell::StaticCell;

static WIFI_CONTROLLER: StaticCell<EspWifiController> = StaticCell::new();
static AP_STACK_RESOURCES: StaticCell<StackResources<3>> = StaticCell::new();
static STA_STACK_RESOURCES: StaticCell<StackResources<3>> = StaticCell::new();

#[esp_hal_embassy::main]
async fn main(spawner: Spawner) {
    let config = esp_hal::Config::default().with_cpu_clock(CpuClock::max());
    let peripherals = esp_hal::init(config);

    /* BEGIN PERIPHERAL SETUP */

    // TIMERS

    // ESP32C6 has two timergroups each having
    // 1 general purpose timer (gpt) and 1 watch dog timer (wdt)
    // There is also the system timer with higher percision.

    // We will use the system timer for embassy and the two GPTs
    // for the sensor and wifi.

    let timg0 = TimerGroup::new(peripherals.TIMG0);
    let timg1 = TimerGroup::new(peripherals.TIMG1);
    let timsys = SystemTimer::new(peripherals.SYSTIMER);

    // RNG
    // Random Number Generator peripheral is used to generate random numbers
    // and is neccesarry for setting up WiFi.

    let mut rng = Rng::new(peripherals.RNG);

    /* END PERIPHERAL SETUP */

    // Heap == 72 * 1024
    esp_alloc::heap_allocator!(72 * 1024);

    /* Begin WiFi Setup */

    let wifi_controller =
        &*WIFI_CONTROLLER.init(esp_wifi::init(timg0.timer0, rng, peripherals.RADIO_CLK).unwrap());

    let ap_resources = AP_STACK_RESOURCES.init(StackResources::new());
    let sta_resources = STA_STACK_RESOURCES.init(StackResources::new());

    let (wifi_ap, wifi_sta, mut controller) =
        esp_wifi::wifi::new_ap_sta(wifi_controller, peripherals.WIFI).unwrap();

    esp_hal_embassy::init(timsys.alarm0);

    let ap_config = embassy_net::Config::ipv4_static(StaticConfigV4 {
        address: Ipv4Cidr::new(Ipv4Addr::new(192, 168, 4, 1), 24),
        gateway: Some(Ipv4Addr::new(192, 164, 4, 1)),
        dns_servers: Default::default(),
    });

    let sta_config = embassy_net::Config::dhcpv4(Default::default());

    let rng_seed = (rng.random() as u64) << 32 | rng.random() as u64;

    let (ap_stack, ap_runner) = embassy_net::new(wifi_ap, ap_config, ap_resources, rng_seed);
    let (sta_stack, sta_runner) = embassy_net::new(wifi_sta, sta_config, sta_resources, rng_seed);

    let conf = Configuration::Mixed(
        ClientConfiguration {
            ..Default::default()
        },
        AccessPointConfiguration {
            ssid: "esp-wifi".try_into().unwrap(),
            ..Default::default()
        },
    );

    controller.set_configuration(&conf).unwrap();
    spawner.spawn(dhcp_server(ap_stack)).ok();
    spawner.spawn(connection(controller)).ok();
    spawner.spawn(ap_task(ap_runner)).ok();

    /* End WiFi Setup */

    //Enable logging according to ESP_LOG Environment Variable
    esp_println::logger::init_logger_from_env();
}

#[embassy_executor::task]
async fn connection(mut controller: WifiController<'static>) {
    info!("start connection task");
    info!("Device capabilities: {:?}", controller.capabilities());

    info!("Starting wifi");
    controller.start_async().await.unwrap();
    info!("Wifi started!");

    loop {
        match esp_wifi::wifi::ap_state() {
            WifiState::ApStarted => {
                info!("Ap has started!");
                controller.wait_for_event(WifiEvent::ApStaconnected).await;
                info!("STA connected");
            }
            WifiState::StaConnected => {
                info!("Station has connected");
            }
            WifiState::Invalid => {
                info!("Invalid")
            }
            e => info!("Other {:#?}", e),
        }
        TaskTimer::after_millis(500).await;
    }
}

#[embassy_executor::task]
async fn dhcp_server(stack: Stack<'static>) {
    let config = DhcpServerConfig {
        ip: Ipv4Addr::new(192, 168, 4, 1),
        lease_time: Duration::from_secs(3600),
        gateways: &[],
        subnet: None,
        dns: &[],
    };

    let mut leaser = SimpleDhcpLeaser {
        start: Ipv4Addr::new(192, 168, 4, 50),
        end: Ipv4Addr::new(192, 168, 4, 200),
        leases: Default::default(),
    };
    let _ = esp_hal_dhcp_server::run_dhcp_server(stack, config, &mut leaser).await;
}

#[embassy_executor::task]
async fn ap_task(mut runner: Runner<'static, WifiDevice<'static, WifiApDevice>>) {
    runner.run().await
}
