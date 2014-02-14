#ifndef __BIT_OPERATIONS_H
#define __BIT_OPERATIONS_H

#define bit_set(reg,bit)   (reg |=  (1<<bit))
#define bit_clr(reg,bit)   (reg &= ~(1<<bit))
#define bit_test(reg,bit)  (reg & (1<<bit))

#define _set_bit_log1(port,bit)     do{PORT##port  |= (1<<bit);   }while(0)  /* output with log "1" */
#define _set_bit_log0(port,bit)     do{PORT##port  &= ~(1<<bit);  }while(0)  /* output with log "0" */
#define _set_bit_as_out(port,bit)   do{DDR##port   |= (1<<bit);   }while(0)  /* output with log "1" */
#define _get_bit_state(port,bit)    (PIN##port & (1<<bit))



#endif
