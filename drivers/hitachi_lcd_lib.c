#include <stdint.h>
#include <intrinsics.h>
#include "hitachi_lcd_lib.h"
#include "systimer.h"
#include "bit.h"


#define DRAM_ADDR(addr)    (0x80|(addr))
#define CGRAM_ADDR(addr)   (0x40|((addr)&0x7F))

#define __set(port,pin) (port |=  (1<<pin))
#define __clr(port,pin) (port &= ~(1<<pin))

#define __lcd_light_on()   __set(LCD,LIGHT)
#define __lcd_light_off()  __clr(LCD,LIGHT)
#define __lcd_rs_on()   __set(LCD,RS)
#define __lcd_rs_off()  __clr(LCD,RS)
#define __lcd_e_on()    __set(LCD,E)
#define __lcd_e_off()   __clr(LCD,E)
#define __lcd_rw_on()   __set(LCD,RW)
#define __lcd_rw_off()  __clr(LCD,RW)
#define __lcd_DB7_on()  __set(LCD,DB7)
#define __lcd_DB7_off() __clr(LCD,DB7)
#define __lcd_DB6_on()  __set(LCD,DB6)
#define __lcd_DB6_off() __clr(LCD,DB6)
#define __lcd_DB5_on()  __set(LCD,DB5)
#define __lcd_DB5_off() __clr(LCD,DB5)
#define __lcd_DB4_on()  __set(LCD,DB4)
#define __lcd_DB4_off() __clr(LCD,DB4)
#define __lcd_delay_us(us) delay_us(us)


__flash uint8_t init_data[]={0x03, 0x03, 0x03, 0x02, 0x02, 0x08, 0x00, 0x08, 0x00, 0x06, 0x00, 0x0C, 0x00, 0x01};
static uint8_t * pLcdBuffer;
static uint8_t __flash * CustomSymbol[8];
static uint8_t NewCustomSymbolFlags;
static uint8_t CharNumber;

static struct{
   uint8_t  DRAMAddrSet    :1,
            CGRAMAddrSet   :1,
            CGRAMUsed      :1,
  bit3:1,
  bit4:1,
  bit5:1,
  bit6:1,
  bit7:1;
}__lcd_flags;

#define get_flag(x)  (__lcd_flags.##x)
#define set_flag(x)  (__lcd_flags.##x = 1)
#define clr_flag(x)  (__lcd_flags.##x = 0)

void __lcd_out(uint8_t data){
   __lcd_e_on();
   (data & 0x01) ? (__lcd_DB4_on()) : (__lcd_DB4_off());
   (data & 0x02) ? (__lcd_DB5_on()) : (__lcd_DB5_off());
   (data & 0x04) ? (__lcd_DB6_on()) : (__lcd_DB6_off());
   (data & 0x08) ? (__lcd_DB7_on()) : (__lcd_DB7_off());
   __lcd_e_off();
}

void __lcd_ports_init(void){
   __lcd_light_on();
   __lcd_rs_off();
   __lcd_rw_off();
   __lcd_e_off();   
   __set(LCD_DDR,RS);
   __set(LCD_DDR,E);
   __set(LCD_DDR,RW);
   __set(LCD_DDR,DB7);
   __set(LCD_DDR,DB6);
   __set(LCD_DDR,DB5);
   __set(LCD_DDR,DB4);
   __set(LCD_DDR,LIGHT);
}

void hitachi_lcd_init(uint8_t * pBuffer){
   uint8_t cnt;
   __lcd_ports_init();
   pLcdBuffer = pBuffer;
   for(cnt = 0; cnt < CHAR_ON_LCD; cnt++){
      pBuffer[cnt] = ' ';
   }
   CharNumber = 0;
   NewCustomSymbolFlags = 0;
   clr_flag(DRAMAddrSet);
   clr_flag(CGRAMAddrSet);
   clr_flag(CGRAMUsed);
   __lcd_delay_us(20000);
   for(cnt = 0; cnt < sizeof(init_data); cnt++){
      __lcd_out(init_data[cnt]);
      __lcd_delay_us(40);
   }
   __lcd_delay_us(2000);
}

void hitachi_lcd_write_data(uint8_t data){
   __lcd_rw_off();
   __lcd_rs_on();
   
   __lcd_out(data>>4);
   
   __lcd_out(data);
}

void hitachi_lcd_set_address(uint8_t addr){
   __lcd_rw_off();
   __lcd_rs_off();
      
   __lcd_out(addr>>4);
   
   __lcd_out(addr);
}

void hitachi_lcd_service(void){   
   if(NewCustomSymbolFlags){
      set_flag(CGRAMUsed);
      static uint8_t DataByteNumber = 0, CustomSymbolNumber = 0;
      if(get_flag(CGRAMAddrSet)){
         hitachi_lcd_write_data(*(CustomSymbol[CustomSymbolNumber] + DataByteNumber));
         if(++DataByteNumber > 7){
            bit_clr(NewCustomSymbolFlags, CustomSymbolNumber);
            clr_flag(CGRAMAddrSet);
            DataByteNumber = 0;
            CustomSymbolNumber = 0;
         }
      } else {
         while(!(NewCustomSymbolFlags & (1<<CustomSymbolNumber)))
            CustomSymbolNumber++;
         hitachi_lcd_set_address(CGRAM_ADDR(CustomSymbolNumber*8));
         set_flag(CGRAMAddrSet);
      }
   } else {
      if(get_flag(CGRAMUsed)){
         clr_flag(CGRAMUsed);
         clr_flag(DRAMAddrSet);
         CharNumber = 0;
         hitachi_lcd_set_address(DRAM_ADDR(0x00));
      } else {
         switch(CharNumber){
           case CHAR_IN_ROW:
            if(get_flag(DRAMAddrSet)){
               clr_flag(DRAMAddrSet);
               hitachi_lcd_write_data(pLcdBuffer[CharNumber]);
               CharNumber++;
            } else {
               set_flag(DRAMAddrSet);
               hitachi_lcd_set_address(DRAM_ADDR(0x40));
            }
            break;
            
           case CHAR_ON_LCD:
            if(get_flag(DRAMAddrSet)){
               clr_flag(DRAMAddrSet);
               CharNumber = 0;
               hitachi_lcd_write_data(pLcdBuffer[CharNumber]);
               CharNumber++;
            } else {
               set_flag(DRAMAddrSet);
               hitachi_lcd_set_address(DRAM_ADDR(0x00));
            }
            break;
            
           default:
            hitachi_lcd_write_data(pLcdBuffer[CharNumber]);
            CharNumber++;
            break;
         }
      }
   }
}

void hitachi_lcd_cgram_symbol(uint8_t __flash * symbol, uint8_t addr){
   CustomSymbol[addr] = symbol;
   bit_set(NewCustomSymbolFlags, addr);
}
