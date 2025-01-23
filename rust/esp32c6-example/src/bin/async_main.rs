#![no_std]
#![no_main]

use embassy_executor::Spawner;
use embassy_time::{Duration, Timer};
use esp_hal::{
    clock::CpuClock,
    gpio::{Level, Output},
    timer::timg::TimerGroup,
};
use {defmt_rtt as _, esp_backtrace as _};

#[embassy_executor::task(pool_size = 3)]
async fn blink_led(mut led: Output<'static>, duration_on: Duration, duration_off: Duration) {
    loop {
        led.set_high();
        Timer::after(duration_on).await;
        led.set_low();
        Timer::after(duration_off).await;
    }
}

#[esp_hal_embassy::main]
async fn main(spawner: Spawner) {
    let config = esp_hal::Config::default().with_cpu_clock(CpuClock::max());
    let peripherals = esp_hal::init(config);

    let timg0 = TimerGroup::new(peripherals.TIMG0);
    esp_hal_embassy::init(timg0.timer0);

    let blue = Output::new(peripherals.GPIO8, Level::Low);
    let green = Output::new(peripherals.GPIO10, Level::Low);
    let red = Output::new(peripherals.GPIO11, Level::Low);

    spawner
        .spawn(blink_led(
            blue,
            Duration::from_secs(1),
            Duration::from_secs(1),
        ))
        .unwrap();

    spawner
        .spawn(blink_led(
            green,
            Duration::from_secs(1),
            Duration::from_secs(2),
        ))
        .unwrap();

    spawner
        .spawn(blink_led(
            red,
            Duration::from_secs(2),
            Duration::from_secs(2),
        ))
        .unwrap();
}
