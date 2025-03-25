#include <avr/io.h>
#include "utils/utils.h"

void changeLevel(int pin, int level) {
	if (level == HIGH) {
		PORTB |= (1 << pin);
	} else if (level == LOW) {
		PORTB &= ~(1 << pin);
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