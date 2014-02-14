#ifndef RELAY_H
#define RELAY_H

#include <ioavr.h>
#include "bit.h"

#define PUMP      Relay1           /* Attaching the MCU outputs to system variables */
#define HEATER    Relay2
#define FAN       Relay3

#define Relay1 D,4
#define Relay2 D,5
#define Relay3 D,6
#define Relay4 D,7

#define switch_on(load)    _set_bit_log1(load)
#define switch_off(load)   _set_bit_log0(load)
#define get_state(load)    _get_bit_state(load)
#define set_as_out(load)   _set_bit_as_out(load)

void relay_lines_init(void);

#endif
