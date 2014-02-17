/* twi_hw.h

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


#ifndef TWI_HW_H_
#define TWI_HW_H_

//#include "hardware.h"
//#include <util/twi.h>
#include <ioavr.h>
#include <stdint.h>

#define TWI_ACK   (1<<TWEA)
#define TWI_NACK  (0)
#define PRESCALER_MASK 0x03

// TWSR values
// (taken from avr-libc twi.h - thank you Marek Michalkiewicz)
// Master
#define TW_START				0x08
#define TW_REP_START				0x10
// Master Transmitter
#define TW_MT_SLA_ACK				0x18
#define TW_MT_SLA_NACK				0x20
#define TW_MT_DATA_ACK				0x28
#define TW_MT_DATA_NACK				0x30
#define TW_MT_ARB_LOST				0x38
// Master Receiver
#define TW_MR_ARB_LOST				0x38
#define TW_MR_SLA_ACK				0x40
#define TW_MR_SLA_NACK				0x48
#define TW_MR_DATA_ACK				0x50
#define TW_MR_DATA_NACK				0x58
// Slave Transmitter
#define TW_ST_SLA_ACK				0xA8
#define TW_ST_ARB_LOST_SLA_ACK		        0xB0
#define TW_ST_DATA_ACK				0xB8
#define TW_ST_DATA_NACK				0xC0
#define TW_ST_LAST_DATA				0xC8
// Slave Receiver
#define TW_SR_SLA_ACK				0x60
#define TW_SR_ARB_LOST_SLA_ACK		        0x68
#define TW_SR_GCALL_ACK				0x70
#define TW_SR_ARB_LOST_GCALL_ACK	        0x78
#define TW_SR_DATA_ACK				0x80
#define TW_SR_DATA_NACK				0x88
#define TW_SR_GCALL_DATA_ACK		        0x90
#define TW_SR_GCALL_DATA_NACK		        0x98
#define TW_SR_STOP				0xA0
// Misc
#define TW_NO_INFO				0xF8
#define TW_BUS_ERROR				0x00
// Address masking value for read/write operations
#define TW_READ                                 1
#define TW_WRITE                                0

extern uint8_t twi_last_error;

typedef enum{TWI_ERROR, TWI_OK}twi_status_t;

void twi_init(uint32_t bitrate_hz);

twi_status_t twi_start(void);
twi_status_t twi_restart(void);
void twi_stop(void);

twi_status_t twi_address_slave(uint8_t addr);
twi_status_t twi_write_slave(uint8_t data);
uint8_t twi_read_slave(uint8_t ack);


#endif /* TWI_HW_H_ */
