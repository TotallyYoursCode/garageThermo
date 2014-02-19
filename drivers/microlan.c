#include <ioavr.h>
#include <stdint.h>

#include "atomic.h"
#include "systimer.h"
#include "microlan.h"

#define _SetLine(port,bit)    do{DDR##port  &= ~(1<<bit); PORT##port |= (1<<bit); }while(0)  /* input with pullup */
#define _ClrLine(port,bit)    do{PORT##port &= ~(1<<bit); DDR##port  |= (1<<bit); }while(0)  /* output with log "0" */
#define _GetLine(port,bit)    (PIN##port & (1<<bit))
#define _StrongPullupOn(port,bit)   do{DDR##port &= ~(1<<bit);  PORT##port  |= (1<<bit); DDR##port |= (1<<bit);}while(0)
#define _StrongPullupOff(port,bit)  do{DDR##port &= ~(1<<bit);  PORT##port  |= (1<<bit); }while(0)

#define set_line(line)         _SetLine(line)
#define clr_line(line)         _ClrLine(line)
#define get_line(line)         _GetLine(line)
#define strong_pullup_on(line)  _StrongPullupOn(line)
#define strong_pullup_off(line) _StrongPullupOff(line)

/* ROM Commands */
#define  SEARCH_ROM        0xF0
#define  READ_ROM          0x33
#define  MATCH_ROM         0x55
#define  SKIP_ROM          0xCC
#define  ALARM_SEARCH      0xEC

/* Function Commands */
#define  CONVERT_T         0x44
#define  WRITE_SCRATCHPAD  0x4E
#define  READ_SCRATCHPAD   0xBE
#define  COPY_SCRATCHPAD   0x48
#define  RECALL_EE         0xB8
#define  READ_POWER_SUPPLY 0xB4

/* Thermometer resolution config */
#define  TEMP_9BIT_RES     0x1F
#define  TEMP_10BIT_RES    0x3F
#define  TEMP_11BIT_RES    0x5F
#define  TEMP_12BIT_RES    0x7F

/* Family Codes */
#define  FAMILY_CODE_DS18B20  0x28
#define  FAMILY_CODE_DS18S20  0x10


void microlan_lines_init(void){
   set_line(line0);
   set_line(line1);
   set_line(line2);
   set_line(line3);
}

void clr_selected_lines(uint8_t LinesMask){
   if(LinesMask & LINE0_MASK) clr_line(line0);
   if(LinesMask & LINE1_MASK) clr_line(line1);
   if(LinesMask & LINE2_MASK) clr_line(line2);
   if(LinesMask & LINE3_MASK) clr_line(line3);
}

void set_selected_lines(uint8_t LinesMask){
   if(LinesMask & LINE0_MASK) set_line(line0);
   if(LinesMask & LINE1_MASK) set_line(line1);
   if(LinesMask & LINE2_MASK) set_line(line2);
   if(LinesMask & LINE3_MASK) set_line(line3);
}

uint8_t get_selected_lines(uint8_t LinesMask){  /* возвращает состояние запрошенных линий */   
   uint8_t result = 0;
   if(LinesMask & LINE0_MASK)
      if(get_line(line0))  result |= LINE0_MASK;   
   if(LinesMask & LINE1_MASK)
      if(get_line(line1))  result |= LINE1_MASK;   
   if(LinesMask & LINE2_MASK)
      if(get_line(line2))  result |= LINE2_MASK;   
   if(LinesMask & LINE3_MASK)
      if(get_line(line3))  result |= LINE3_MASK;
   return result;   
}

void __ml_write1(uint8_t LinesMask){
   __atomic_block_start();
   clr_selected_lines(LinesMask);
   delay_us(6);
   set_selected_lines(LinesMask);
   __atomic_block_end();   
   delay_us(64);
}

void __ml_write0(uint8_t LinesMask){
   __atomic_block_start();
   clr_selected_lines(LinesMask);
   delay_us(60);
   set_selected_lines(LinesMask);
   __atomic_block_end();   
   delay_us(10);
}

uint8_t __ml_read(uint8_t LinesMask){
   uint8_t result;
   __atomic_block_start();
   clr_selected_lines(LinesMask);
   delay_us(6);
   set_selected_lines(LinesMask);
   delay_us(9);
   result = get_selected_lines(LinesMask);
   __atomic_block_end();
   delay_us(55);
   return result;
}

uint8_t __ml_reset(uint8_t LinesMask){ /* возвращает единицы в линиях без датчиков */
   uint8_t result;
   clr_selected_lines(LinesMask);
   delay_us(240);
   delay_us(240);
   __atomic_block_start();
   set_selected_lines(LinesMask);
   delay_us(70);
   result = get_selected_lines(LinesMask);
   __atomic_block_end();
   delay_us(205);
   delay_us(205);
   return result;
}

void __ml_write_byte(uint8_t Byte, uint8_t LinesMask){
   uint8_t mask;
   for(mask = 1; mask > 0; mask <<= 1){
      if(Byte & mask)
         __ml_write1(LinesMask);
      else
         __ml_write0(LinesMask);
   }
}

void __ml_read_byte(uint8_t * Bytes, uint8_t LinesMask){
   uint8_t Bit,Lines,*bytes;
   bytes = Bytes;
   for(Lines = 0; Lines < 4; Lines++)
      *bytes++ = 0;
   for(Bit = 0; Bit < 8; Bit++){
      Lines = __ml_read(LinesMask);
      if(Lines & LINE0_MASK)  *(Bytes  ) |= (1<<Bit);
      if(Lines & LINE1_MASK)  *(Bytes+1) |= (1<<Bit);
      if(Lines & LINE2_MASK)  *(Bytes+2) |= (1<<Bit);
      if(Lines & LINE3_MASK)  *(Bytes+3) |= (1<<Bit);
   }
}
   
uint8_t __ml_crc8(uint8_t inData, uint8_t seed){
   uint8_t bitsLeft, temp;
   for (bitsLeft = 8; bitsLeft > 0; bitsLeft--){
      temp = ((seed ^ inData) & 0x01);
      if (temp == 0)
         seed >>= 1;
      else{
         seed ^= 0x18;
         seed >>= 1;
         seed |= 0x80;
      }
      inData >>= 1;
   }
   return seed;
}


uint8_t start_converting_temp(uint8_t LinesMask){
   uint8_t lines;
   lines = __ml_reset(LinesMask);   /* получение единиц в линиях без ответа */
   lines = (~lines)&(LinesMask);    /* получение единиц в линиях, где есть датчики из списка запрошенных */
   if(lines == 0) return 0;
   __ml_write_byte(SKIP_ROM, lines);   /* дальнейшая работа только с активными линиями из списка запрошенных */
   __ml_write_byte(CONVERT_T, lines);
   return lines;
}

uint8_t try_to_get_temp(microlan_data_t * TempData, uint8_t LinesMask){
   uint8_t lines,tmplines,byteN,lineN,linesbytes[4]={0,0,0,0},crc8[4]={0,0,0,0};
   
   lines = get_selected_lines(LinesMask);             /* получение единиц в каналах с завершенным преобразованием */
   if(lines == LinesMask){                            /* преобразование завершено во всех запрошенных каналах */
      lines = __ml_reset(LinesMask);                  /* получение единиц в линиях без ответа */
      lines = (~lines)&(LinesMask);                   /* получение единиц в линиях, где есть датчики из списка запрошенных */
      __ml_write_byte(SKIP_ROM, lines);               /* команда пропуска ROM кода */
      __ml_write_byte(READ_SCRATCHPAD, lines);        /* команда чтения ОЗУ термодатчика */
      tmplines = lines;
      for(byteN = 0; byteN < 9; byteN++){             /* заводим цикл на чтение всех 9 байт ОЗУ */
         __ml_read_byte(linesbytes, lines);           /* считываем запрошенные линии, результат возвращается в массив linesbytes[4] */
         for(lineN = 0; lineN < 4; lineN++){          /* перебираем 4 линии */
            if(lines & (1<<0)){
               (*(TempData+lineN)).Scratchpad.Bytes[byteN] = linesbytes[lineN];  /* сохраняем каждый байт */
               crc8[lineN] = __ml_crc8(linesbytes[lineN],crc8[lineN]);           /* одновременно считаем CRC8 */
            }
            lines >>= 1;
         }
         lines = tmplines;
      }
      lines = 0;                                      /* сбрасываем маску ответивших линий */
      for(lineN = 0; lineN <4; lineN++){              /* проверяем полученную контрольную сумму для всех запрошенных линий */
         if(tmplines & (1<<0))                           /* линия была запрошена? */
            if(crc8[lineN] == 0)                         /* контрольная сумма сошлась? */
               lines |= (1<<lineN);                         /* ставим бит валидности */
         tmplines >>= 1;
      }
      return lines;                                   /* возвращаем маску валидных данных ответивших каналов */
   } else {
      return 0;
   }
}


   
