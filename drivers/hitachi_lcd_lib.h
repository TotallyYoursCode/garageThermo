#ifndef __HITACHI_LCD_LIB
#define __HITACHI_LCD_LIB

#include "ioavr.h"

#define LCD    PORTA       /* lcd port */
#define LCD_DDR DDRA        /* lcd direction port */
#define DB7    7           /* number of pin which is connected to DB7 */
#define DB6    6           /* number of pin which is connected to DB6 */
#define DB5    5           /* number of pin which is connected to DB5 */
#define DB4    4           /* number of pin which is connected to DB4 */
#define E      3           /* strobe signal */
#define RW     2           /* read/write signal */
#define RS     1           /* address/data signal */
#define LIGHT  0           /* when bit is set to "1", backlight turning on */

#define CHAR_IN_ROW              16
#define CHAR_ON_LCD              32
#define LCD_REFRESH_FREQUENCY    50

#define LCD_ROWS                 (CHAR_ON_LCD/CHAR_ON_LCD)
#define LCD_REFRESH_STEPS        (CHAR_ON_LCD + LCD_ROWS)
#define LCD_SERVICE_FREQUENCY    (LCD_REFRESH_FREQUENCY*LCD_REFRESH_STEPS)
#define LCD_SERVICE_PERIOD_us    (1000000UL/LCD_SERVICE_FREQUENCY)
#define LCD_SERVICE_PERIOD_ms    ((LCD_SERVICE_PERIOD_us+500)/1000)

void hitachi_lcd_init(uint8_t * pBuffer);
void hitachi_lcd_service(void);
void hitachi_lcd_cgram_symbol(uint8_t __flash * symbol, uint8_t addr);

#endif