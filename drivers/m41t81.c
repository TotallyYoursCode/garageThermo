/* m41t81.c

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
 *
 *
 * mktime() function based on newlib mktime.c Author: G. Haley
 *
 */

#include "m41t81.h"
#include <string.h>


#define M41_addr 0xD0

uint8_t m41_control=0;
volatile static uint8_t m41_stop=0;

tm time;

static void mktime ();
static void m41_parse(const uint8_t addr,const uint8_t data);

static const int DAYS_IN_MONTH[12] ={31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
#define _DAYS_IN_MONTH(x) ((x == 1) ? days_in_feb : DAYS_IN_MONTH[x])
static const int _DAYS_BEFORE_MONTH[12] ={0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
#define _ISLEAP(y) (((y) % 4) == 0 && (((y) % 100) != 0 || (((y)) % 400) == 0))
#define _DAYS_IN_YEAR(year) (_ISLEAP(year) ? 366 : 365)

hw_ret_status time_init() {
	/*Очистим HALT*/
	twi_start();
	twi_address_slave(M41_addr|TW_WRITE);
	twi_write_slave(0x0c);
	twi_write_slave(0);
	twi_stop();
        
	twi_start();
	twi_address_slave(M41_addr|TW_WRITE);
	twi_write_slave(0x0a);
	twi_write_slave(0x40);
	twi_stop();
        
	twi_start();
	twi_address_slave(M41_addr|TW_WRITE);
	twi_write_slave(0x13);
	twi_write_slave(0xf0);
	twi_stop();
	return time_read();
}

hw_ret_status m41start(){
	twi_start();
	twi_address_slave(M41_addr|TW_WRITE);
	twi_write_slave(0x01);
	twi_write_slave(time.sec&0x7f);
	twi_stop();
	time.sec=0;
	return SUCCESS;
}

hw_ret_status time_read() {
	uint8_t i;

	/*Прочитаем текущее время*/
	twi_start();
	twi_address_slave(M41_addr|TW_WRITE);
	twi_write_slave(0x01);
	twi_restart();        
	twi_address_slave(M41_addr|TW_READ);
	for (i=1;i<8;i++) {
		m41_parse(i,twi_read_slave(TWI_ACK));
	}
	m41_parse(8,twi_read_slave(TWI_ACK));
	twi_stop();
	mktime();
	if (m41_stop) {
		m41_stop=0;
		m41start();
		return CLOCK_STOP;
	}
#ifdef DBG_M41
	itoa(time.hour,debug_str,10);
	debug(debug_str);
	debug(":");
	itoa(time.min,debug_str,10);
	debug(debug_str);
	debug(":");
	itoa(time.sec,debug_str,10);
	debug(debug_str);
	debug(">>");
	itoa(time.unix_time,debug_str,16);
	debug(debug_str);
	debug("\r\n");
#endif
	return SUCCESS;
}

static void m41_parse(const uint8_t addr,const uint8_t data) {
	switch (addr) {
	case 1:
		if (data&0x80) m41_stop=1;
		time.sec=((data&0x70)>>4)*10 + (data&0x0f);
		if (time.sec>59) time.sec=0;
		break;
	case 2:
		time.min=((data&0x70)>>4)*10 + (data&0x0f);
		if (time.min>59) time.min=0;
		break;
	case 3:
		time.hour=((data&0x30)>>4)*10 + (data&0x0f);
		if (time.hour>23) time.hour=0;
		break;
	case 5:
		time.day=((data&0x30)>>4)*10 + (data&0x0f);
		break;
	case 6:
		time.month=(data&0x0f);
		if (data&0x10) time.month+=10;
		if (time.month>12) time.month=1;
		break;
	case 7:
		time.year=((data&0xf0)>>4)*10 + (data&0x0f);
		if (time.year>99) time.year=0;
		if (check_date()) time.day=1;
		break;
	case 8:
//		m41_control=data;
		break;
	default:
		break;
	}
}

uint8_t check_date(){
	uint8_t days_in_feb=28;
	if (_DAYS_IN_YEAR(time.year+2000)==366) days_in_feb=29;
	if (time.day>_DAYS_IN_MONTH(time.month-1)) return _DAYS_IN_MONTH(time.month-1);
	return 0;
}

static void mktime () {

#define _SEC_IN_MINUTE 60L
#define _SEC_IN_HOUR 3600L
#define _SEC_IN_DAY 86400L

	uint32_t tim = 0;
	long days = 0;
	uint8_t year;

	/* Считаем секунды за указанный день */
	tim += time.sec + (time.min * _SEC_IN_MINUTE) +
			(time.hour * _SEC_IN_HOUR);

	/* Считаем дни в указанном году */
	days += time.day - 1;
	days += _DAYS_BEFORE_MONTH[(time.month-1)];
	if ((time.month-1) > 1 && _DAYS_IN_YEAR (time.year+2000) == 366) days++;

	/*Считаем дней до указанного года, от 2000*/
	for (year=0;year < time.year; year++) days += _DAYS_IN_YEAR (year+2000);

	/* Вычисляем день недели */
	time.day_w = (uint8_t)((days+6) % 7);

	/* Вычисляем сколько всего секунд прошло с 01.01.2000 */
	tim += (days * _SEC_IN_DAY);

	time.position=(tim/1800) % 96;
	time.unix_time=tim;
}

hw_ret_status m41stop(){
	twi_start();
	twi_address_slave(M41_addr|TW_WRITE);
	twi_write_slave(0x01);
	twi_write_slave(0x80);
	twi_stop();
	time.sec=0;
	return SUCCESS;
}

hw_ret_status m41calibrate(){
	if (m41_control==0x20) m41_control=0;
	twi_start();
	twi_address_slave(M41_addr|TW_WRITE);
	twi_write_slave(0x08);
	twi_write_slave(m41_control);
	twi_stop();
	return SUCCESS;
}

hw_ret_status m41set_time(){
	twi_start();
	twi_address_slave(M41_addr|TW_WRITE);
	twi_write_slave(0x01);

	twi_write_slave(((time.sec/10)<<4)|(time.sec%10));
	twi_write_slave(((time.min/10)<<4)|(time.min%10));
	twi_write_slave(((time.hour/10)<<4)|(time.hour%10));
	twi_write_slave(1);
	twi_write_slave(((time.day/10)<<4)|(time.day%10));
	twi_write_slave(((time.month/10)<<4)|(time.month%10));
	twi_write_slave(((time.year/10)<<4)|(time.year%10));
	twi_stop();
	return SUCCESS;
}

uint8_t m41_get_control(){
	twi_start();
	twi_address_slave(M41_addr|TW_WRITE);
	twi_write_slave(0x08);
	twi_restart();
        
	twi_address_slave(M41_addr|TW_READ);
	m41_parse(8,twi_read_slave(TWI_NACK));
	twi_stop();
	return TWDR;
}

#define M41_ADDR 0xD0
#define M41_STOPPED_MASK 0x80
//static clock_t time;
static struct{
   uint8_t stopped:1,
 bit1:1,
 bit2:1,
 bit3:1,
 bit4:1,
 bit5:1,
 bit6:1,
 bit7:1;
}m41t81_state;
 
void m41t81_data_parse(uint8_t addr, uint8_t data);

uint8_t m41t81_get_data(void){
   uint8_t i;   
   /*Прочитаем текущее время*/
   twi_start();
   twi_address_slave(M41_ADDR|TW_WRITE);
   twi_write_slave(0x00);
   twi_restart();
   twi_address_slave(M41_ADDR|TW_READ);
   for(i=1;i<20;i++){
      m41t81_data_parse(i,twi_read_slave(TWI_ACK));
   }
   twi_stop();
   return SUCCESS;
}

void m41t81_data_parse(uint8_t addr, uint8_t data){
   switch(addr){
    case 1:
      if (data & M41_STOPPED_MASK) m41t81_state.stopped = 1;
      time.sec=((data&0x70)>>4)*10 + (data&0x0f);
      break;
      
    case 2:
      time.min=((data&0x70)>>4)*10 + (data&0x0f);
      break;
      
    case 3:
      time.hour=((data&0x30)>>4)*10 + (data&0x0f);
      break;
      
    case 5:
      time.day=((data&0x30)>>4)*10 + (data&0x0f);
      break;
      
    case 6:
      time.month=((data&0x10)>>4)*10 + (data&0x0f);
      break;
      
    case 7:
      time.year=((data&0xf0)>>4)*10 + (data&0x0f);
      break;
    case 8:
      //		m41_control=data;
      break;
    default:
      break;
   }
}
/* Data bytes */
/*
1st byte:            tenths/hundredths of a second register
2nd byte:            seconds register
3rd byte:            minutes register
4th byte:            century/hours register
5th byte:            day register
6th byte:            date register
7th byte:            month register
8th byte:            year register
9th byte:            control register
10th byte:           watchdog register
11th - 16th bytes:   alarm registers
17th - 19th bytes:   reserved
20th byte:           square wave register
*/
