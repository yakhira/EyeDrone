#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <avr/sleep.h>
#include <avr/interrupt.h>

#define HIGH 1
#define LOW 0

#define SLEEP_016MS    (0<<WDP3 )|(0<<WDP2 )|(0<<WDP1)|(0<<WDP0)
#define SLEEP_032MS    (0<<WDP3 )|(0<<WDP2 )|(0<<WDP1)|(1<<WDP0)
#define SLEEP_64MS     (0<<WDP3 )|(0<<WDP2 )|(1<<WDP1)|(0<<WDP0)
#define SLEEP_125MS    (0<<WDP3 )|(0<<WDP2 )|(1<<WDP1)|(1<<WDP0)
#define SLEEP_250MS    (0<<WDP3 )|(1<<WDP2 )|(0<<WDP1)|(0<<WDP0)
#define SLEEP_500MS    (0<<WDP3 )|(1<<WDP2 )|(0<<WDP1)|(1<<WDP0)
#define SLEEP_1SEC     (0<<WDP3 )|(1<<WDP2 )|(1<<WDP1)|(0<<WDP0)
#define SLEEP_2SEC     (0<<WDP3 )|(1<<WDP2 )|(1<<WDP1)|(1<<WDP0)
#define SLEEP_4SEC     (1<<WDP3 )|(0<<WDP2 )|(0<<WDP1)|(0<<WDP0)
#define SLEEP_8SEC     (1<<WDP3 )|(0<<WDP2 )|(0<<WDP1)|(1<<WDP0)

void setSleep(int b);
void digitalWrite_A(int pin, int level);
void digitalWrite_B(int pin, int level);

#ifdef __cplusplus
}
#endif
