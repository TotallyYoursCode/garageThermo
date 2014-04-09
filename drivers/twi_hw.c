/* twi_hw.c

Copyright (C) Alex S. (mailto: md5sum@alexsp.ru)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details
http://www.gnu.org/licenses/gpl-2.0.html.

You should have received a copy of the GNU General Public License
along with this program in the file gpl-2.0.txt;
if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "twi_hw.h"
#include "systimer.h"

uint8_t twi_last_error;

#define _twi_wait_operation()       while(!(TWCR&(1<<TWINT)))
#define _twi_wait()                 do{_twi_wait_operation(); twi_last_error = TWSR;}while(0)
#define _check_status_error(a)      if(_twi_check(a)) {_twi_stop(); TWCR=0; return TWI_ERROR;}
#define _twi_start()                do{TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN)|(1<<TWSTO); _twi_wait();}while(0)
#define _twi_restart()              do{TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN); _twi_wait();}while(0)
#define _twi_stop()                 do{TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);}while(0)
#define _twi_send(data,ack)         do{TWDR = data; TWCR = (1<<TWINT)|(1<<TWEN)|ack; _twi_wait();}while(0)
#define _twi_check(a)               ((TWSR&(~PRESCALER_MASK)) == a ? 0 : 1)
#define _twi_set_prescaler(a)       do{TWSR &= (~PRESCALER_MASK); TWSR |= a;}while(0)

void twi_init(uint32_t bitrate_hz){
   if(bitrate_hz >= (F_CPU_HZ/16)){
      uint32_t temp;
      uint8_t twps_value = 1,degree = 0;
      temp = (F_CPU_HZ/2/bitrate_hz) - 8;
      while(temp > (255*twps_value)){
         twps_value *= 4;
         degree++;
      }
      _twi_set_prescaler(degree);
      TWBR = temp/(255*twps_value);
   } else {
      _twi_set_prescaler(0);
      TWBR = 0;
   }
   TWCR = (1<<TWEN);
}


twi_status_t twi_start(void){
   _twi_start();
   _check_status_error(TW_START);
   return TWI_OK;
}

twi_status_t twi_restart(void){
   _twi_restart();
   _check_status_error(TW_REP_START);
   return TWI_OK;   
}

void twi_stop(void){
   TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
}

// 1. addr + write, NACK - адресация слейва с последующей записью
// 2. addr + read,  NACK - адресация слейва с последующим чтением
twi_status_t twi_address_slave(uint8_t addr){
   _twi_send(addr,TWI_NACK);
   _check_status_error(TW_MT_SLA_ACK);
   return TWI_OK;
}


// 3. data,         NACK - запись данных в адресованный слейв
twi_status_t twi_write_slave(uint8_t data){
   _twi_send(data,TWI_NACK);
   _check_status_error(TW_MT_DATA_ACK);
   return TWI_OK;
}


// 4. null(0xff),    ACK - чтение данных адресованного слейва c продолжением
// 5. null(0xff),   NACK - окончание чтения данных адресованного слейва
uint8_t twi_read_slave(uint8_t ack){
   _twi_send(0xFF,ack);
   return TWDR;
}