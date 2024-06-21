#include <Arduino.h>
#include "utils/uart.h"

#define SWITCH_PIN PB4

void setup(void)
{
	pinMode(SWITCH_PIN, OUTPUT);
	digitalWrite(SWITCH_PIN, LOW);
}

void loop() {

	char ch = '\xff';
	
	do {
		ch = uart_getc();
	} while (ch == '\xff');

	if (ch == '\x31'){
		uart_puts("on\r\n");
		digitalWrite(SWITCH_PIN, HIGH);
	}

	if (ch == '\x30'){
		uart_puts("off\r\n");
		digitalWrite(SWITCH_PIN, LOW);
	}
}                    