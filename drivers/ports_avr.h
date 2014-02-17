#ifndef _PORTS_AVR_H
#define _PORTS_AVR_H

#define _SetPinAsOut(port,bit)   do{DDR##port  |=  (1<<bit);}while(0)
#define _SetPinAsInput(port,bit) do{DDR##port  &= ~(1<<bit);}while(0)

#define _ClearPin(port,bit)      do{PORT##port &= ~(1<<bit);}while(0)
#define _SetPin(port,bit)        do{PORT##port |=  (1<<bit);}while(0)

#define _GetPin(port,bit)        (PIN##port & (1<<bit))

#define set_pin_as_out(line)      _SetPinAsOut(line)
#define set_pin_as_input(line)    _SetPinAsInput(line)

#define clear_pin(line)           _ClearPin(line)
#define set_pin(line)             _SetPin(line)

#define switch_pullup_on(line)    _SetPin(line)
#define switch_pullup_off(line)   _ClearPin(line)

#define get_pin(line)             _GetPin(line)

#endif
