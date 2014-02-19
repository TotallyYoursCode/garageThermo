#ifndef BIN2BCD_H
#define BIN2BCD_H

typedef enum{
   NO_FLAGS = 0,
   REPLACE_ZEROES = (1<<7)
}bin_2_ascii_flag_t;


uint8_t * bin_2_ascii(uint8_t * str, int32_t value, uint8_t digits, bin_2_ascii_flag_t flags);
uint8_t * copy_str(uint8_t * dest_str, uint8_t __flash * source_str);
uint8_t * put_str(uint8_t * dest_str, uint8_t __flash * source_str);
uint8_t * put_str_s(uint8_t * dest_str, uint8_t * source_str);
uint8_t * put_char(uint8_t * dest_str, uint8_t _char);

#endif