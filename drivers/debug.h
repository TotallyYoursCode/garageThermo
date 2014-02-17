#ifndef __DEBUG_HEADER_FILE
#define __DEBUG_HEADER_FILE

#include "bit.h"
#include "ioavr.h"

#define DEBUG

#define DEBUG_WIRE    B,7

#define debug_on(signal)    _set_bit_log1(signal)
#define debug_off(signal)   _set_bit_log0(signal)
#define debug_init(signal)  _set_bit_as_out(signal)

#endif
