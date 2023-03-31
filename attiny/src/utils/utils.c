#include <avr/io.h>
#include "utils/utils.h"

void digitalWrite_B(int pin, int level) {
	if (level == HIGH) {
		asm("sbi %0, %1" :: "I" (_SFR_IO_ADDR(PORTB)), "I" (pin));
	} else if (level == LOW) {
		asm("cbi %0, %1" :: "I" (_SFR_IO_ADDR(PORTB)), "I" (pin));
	}
}

void digitalWrite_A(int pin, int level) {
	if (level == HIGH) {
		asm("sbi %0, %1" :: "I" (_SFR_IO_ADDR(PORTB)), "I" (pin));
	} else if (level == LOW) {
		asm("cbi %0, %1" :: "I" (_SFR_IO_ADDR(PORTB)), "I" (pin));
	}
}

void setSleep(int b) {
    {
		ADCSRA &= ~(1<<ADEN);
		ACSR |= (1 << ACD); //Analog comparator off
		ACSR = ADMUX = ADCSRA = 0;
    }

	WDTCR |= b;    //Watchdog
	// Enable watchdog timer interrupts
	WDTCR |= (1 << WDTIE);

    cli(); // No interrupts; timed sequence
    sei(); // Enable global interrupts or we never wake

	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  }