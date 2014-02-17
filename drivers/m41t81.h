/* m41t81.h

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


#ifndef M41T81_H_
#define M41T81_H_

//#include "hardware.h"
#include "twi_hw.h"
//#define DBG_M41 0
typedef enum{SUCCESS=0,ERROR,CRC,SHORT,OPEN,WAIT,CLOCK_STOP} hw_ret_status;

hw_ret_status time_read();
hw_ret_status time_init();
typedef struct{
	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t day_w;
	uint8_t day;
	uint8_t month;
	uint8_t year;
	uint32_t unix_time;
	uint8_t position;
} tm;

typedef struct{
   uint8_t sec,min,hour,day_w,day,month,year;
} clock_t;

//extern uint8_t m41_stop;
extern tm time;
extern uint8_t m41_control;
hw_ret_status m41stop();
hw_ret_status m41calibrate();
hw_ret_status m41set_time();
uint8_t check_date();
uint8_t m41_get_control();

#endif /* M41T81_H_ */
