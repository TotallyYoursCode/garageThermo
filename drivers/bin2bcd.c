#include <stdint.h>

#include "bin2bcd.h"
#include "cyr_decode.h"

__flash uint32_t degree[] = {
   1000000000,
   100000000,
   10000000,
   1000000,
   100000,
   10000,
   1000,
   100,
   10,
   1 };

#define ARR_SIZE  (sizeof(degree)/sizeof(degree[0]))

uint8_t * bin_2_ascii(uint8_t * str, int32_t value, uint8_t digits, uint8_t flags){
   int8_t i;
   uint8_t digit_value = 0, significant_digits = 0, negative = 0;
   if(value < 0){
      value = -value;
      negative = !0;
   }
  
   if(digits > ARR_SIZE)
      digits = ARR_SIZE;
      
   for(i = 0; i <= (ARR_SIZE-1); i++){
      digit_value = 0;
      while(value >= degree[i]){
         value -= degree[i];
         digit_value++;
      }
      *str = digit_value + '0';
      if(!(significant_digits)){
         if(i != (ARR_SIZE-1)){
            if(digit_value == 0){
               if(flags & REPLACE_ZEROES){
                  *str = ' ';
               }
            } else {
               significant_digits = !0;
            }
         }
      }
      if(digits >= (ARR_SIZE-i))
         *str++;
   }
   if(negative){
      if(flags & REPLACE_ZEROES){
         for(i = 1; (*(str-i) != ' ') && (i <= digits) ; i++);
         *(str-i) = '-';
      } else {
         *(str - digits) = '-';
      }
   }
   return str;
}

uint8_t * put_str(uint8_t * dest_str, uint8_t __flash * source_str){
   while(*source_str){
      *dest_str++ = lcd_char_decode(*source_str++);
   }
   return dest_str;
}