#![no_std]

use core::arch::asm;

use esp_hal::{
    delay::Delay,
    gpio::{
        interconnect::{PeripheralInput, PeripheralOutput},
        Input, InputPin, Level, Output, OutputPin, Pull,
    },
    peripheral::Peripheral,
    timer::Timer,
};
use log::info;

pub struct HCSR04<'h, Clock: Timer> {
    trigger: Output<'h>,
    echo: Input<'h>,
    timer: Clock,
}

impl<'h, Clock: Timer> HCSR04<'h, Clock> {
    pub fn new<TRIG: PeripheralOutput + OutputPin, ECHO: PeripheralInput + InputPin>(
        trigger: impl Peripheral<P = TRIG> + 'h,
        echo: impl Peripheral<P = ECHO> + 'h,
        timer: Clock,
    ) -> Self {
        Self {
            trigger: Output::new(trigger, Level::Low),
            echo: Input::new(echo, Pull::Down),
            timer,
        }
    }

    pub fn measure_distance(&mut self) -> f64 {
        let delay = Delay::new();
        self.trigger.set_high();
        delay.delay_micros(10);
        self.trigger.set_low();

        self.timer.start();

        while self.echo.is_low() {
            unsafe { asm!("nop") };
        }

        let start = self.timer.now();

        while self.echo.is_high() {
            unsafe { asm!("nop") };
        }

        let end = self.timer.now();

        let pulse_duration = (end - start).to_micros();

        info!(
            "Pulse duration is End({}) - Start({}) = {}",
            end, start, pulse_duration
        );

        // let distance = (0.0343 * pulse_duration as f64) / 2_f64;
        //or from the manual:
        let distance = pulse_duration as f64 / 58_f64;
        distance
    }
}
