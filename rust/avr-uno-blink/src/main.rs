#![feature(asm_experimental_arch)]
#![no_std]
#![no_main]

use core::panic::PanicInfo;

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    loop {}
}

#[no_mangle]
pub extern fn main() -> ! {
    // Set PB5 (pin 13) as an output
    unsafe {
        // DDRB: Data Direction Register for Port B
        // Set bit 5 to 1 (output mode)
        const DDRB: *mut u8 = 0x24 as *mut u8;
        *DDRB |= 1 << 5;
    }

    loop {
        // Turn LED on (set PB5 high)
        unsafe {
            const PORTB: *mut u8 = 0x25 as *mut u8;
            *PORTB |= 1 << 5;
        }

        delay(1000);

        // Turn LED off (set PB5 low)
        unsafe {
            const PORTB: *mut u8 = 0x25 as *mut u8;
            *PORTB &= !(1 << 5);
        }

        delay(1000);
    }
}

fn delay(ms: u16) {
    for _ in 0..ms {
        for _ in 0..1600 { //Each inner loop iteration takes ~10 cycles
            // Prevent the compiler from optimizing this loop away
            unsafe { core::arch::asm!("nop") };
        }
    }
}
