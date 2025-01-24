#![no_std]
#![no_main]

use embassy_executor::Spawner;
use embassy_time::Timer as TaskTimer;
use esp_backtrace as _;
use esp_hal::{
    clock::CpuClock,
    timer::timg::{Timer, TimerGroup},
};
use log::info;
use rust_iot_app::HCSR04;

#[embassy_executor::task]
async fn sense_distance(hcsr04: HCSR04<'static, Timer>) {
    todo!()
}

#[esp_hal_embassy::main]
async fn main(spawner: Spawner) {
    // generator version: 0.2.2

    let config = esp_hal::Config::default().with_cpu_clock(CpuClock::max());
    let peripherals = esp_hal::init(config);

    esp_println::logger::init_logger_from_env();

    let timg0 = TimerGroup::new(peripherals.TIMG0);
    esp_hal_embassy::init(timg0.timer0);

    let timg1 = TimerGroup::new(peripherals.TIMG1);

    info!("Embassy initialized!");

    let mut hcsr04 = HCSR04::new(peripherals.GPIO3, peripherals.GPIO2, timg1.timer0);

    loop {
        TaskTimer::after_secs(1).await;
        match hcsr04.measure_distance() {
            Ok(e) => info!("Distance measured: {}", e),
            Err(_) => info!("errrrrorrrrr"),
        }
    }
}
