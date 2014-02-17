#ifndef BIN2BCD_H
#define BIN2BCD_H

#define REPLACE_ZEROES     (1<<7)


uint8_t * bin_2_ascii(uint8_t * str, int32_t value, uint8_t digits, uint8_t flags);
uint8_t * put_str(uint8_t * dest_str, uint8_t __flash * source_str);

#endif