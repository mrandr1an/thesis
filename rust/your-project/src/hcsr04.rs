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
use esp_println::println;

/// The HCSR04 requires two pins and a clock to fully function.
/// 1. The Trigger pin which is configured as an output and sends the ultrasonic pulse.
/// 2. The Echo pin which is configured as input and returns high for the duration of the ultrasonic echo.
/// 3. The clock which times the duration of the Echo pins HIGH State.
/// From a design perspective the actual GPIO pins do not matter and must be configured by the user.
/// So in essence:
/// HCSR04 requires:
/// 1. One arbitary Output
/// 2. One arbitary Input
/// 3. One arbitary Timer
pub struct HCSR04<'h, Clock: Timer> {
    trigger: Output<'h>,
    echo: Input<'h>,
    clock: Clock,
}

impl<'h, Clock: Timer> HCSR04<'h, Clock> {
    /// In order to create a compile time safe HCSR04 object
    /// you need to give
    /// - A Peripheral that is configured as an Output and is also an OutputPin
    /// - A Peripheral that is configured as an Input and is also an InputPin
    /// - An object that is able to measure time
    pub fn new<TRIG: PeripheralOutput + OutputPin, ECHO: PeripheralInput + InputPin>(
        trigger: impl Peripheral<P = TRIG> + 'h,
        echo: impl Peripheral<P = ECHO> + 'h,
        timer: Clock,
    ) -> Self {
        Self {
            trigger: Output::new(trigger, Level::Low),
            echo: Input::new(echo, Pull::Down),
            clock: timer,
        }
    }

    pub fn measure_distance(&mut self) -> f64 {
        let delay = Delay::new();

        self.clock.start();
        //Send ultrasonic wave
        self.trigger.set_high();
        delay.delay_micros(10);
        self.trigger.set_low();
        //Ultra sonic wave is sent

        //Wait until echo bounces back
        while self.echo.is_low() {
            //DO NOT DEOPTIMIZE THE LOOP AWAY
            unsafe { asm!("nop") };
        }

        //Start timing start = CURRENT_TIME
        let start = self.clock.now();

        //Wait until echo pin returns to low
        while self.echo.is_high() {
            //DO NOT DEOPTIMIZE THE LOOP AWAY
            unsafe { asm!("nop") };
        }

        //End timing end = CURRENT_TIME > start
        let end = self.clock.now();

        let pulse_duration = (end - start).to_micros();

        //return the distance
        pulse_duration as f64 / 58_f64
    }
}
