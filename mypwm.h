#ifndef MYPWM_H
#define MYPWM_H

#include <application.h>
#include <inttypes.h>

#define PWM_FREQ 50000 // in Hertz (SET YOUR FREQUENCY)

void analogWrite2(uint16_t pin, uint8_t value);

#endif
