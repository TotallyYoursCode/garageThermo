#include "stdint.h"
__flash uint8_t decode_cyrillic_table[64]={
0x41,0xA0,0x42,0xA1,0xE0,0x45,0xA3,0xA4,     /* �������� */
0xA5,0xA6,0x4B,0xA7,0x4D,0x48,0x4F,0xA8,     /* �������� */
0x50,0x43,0x54,0xA9,0xAA,0x58,0xE1,0xAB,     /* �������� */
0xAC,0xE2,0xAD,0xAE,0xAD,0xAF,0xB0,0xB1,     /* �������� */
0x61,0xB2,0xB3,0xB4,0xE3,0x65,0xB6,0xB7,     /* �������� */
0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0x6F,0xBE,     /* �������� */
0x70,0x63,0xBF,0x79,0xE4,0x78,0xE5,0xC0,     /* �������� */
0xC1,0xE6,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7 };   /* �������� */

uint8_t lcd_char_decode(uint8_t c) {
   if(c > 191) {
      c = decode_cyrillic_table[c-192];
   }
   return c;
}