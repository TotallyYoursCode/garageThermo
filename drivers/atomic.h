#ifndef ATOMIC_H
#define ATOMIC_H

#include <intrinsics.h>
#define __atomic_block_start()   do{uint8_t __SREG_tmp = __save_interrupt(); __disable_interrupt()
#define __atomic_block_end()     __restore_interrupt(__SREG_tmp);}while(0)

#define __atomic_block(a)        __atomic_block_start(); a; __atomic_block_end()

#endif
