#ifndef MICROLAN_H
#define MICROLAN_H

/* определение линий microlan */
#define T1ch   LINE0_MASK
#define T2ch   LINE1_MASK
#define T3ch   LINE2_MASK
#define T4ch   LINE3_MASK

#define CONVERSION_TIME_ms    750
#define THERMOMETERS_POLL_PERIOD_ms    1000

#define LINE0_MASK   (1<<0)
#define LINE1_MASK   (1<<1)
#define LINE2_MASK   (1<<2)
#define LINE3_MASK   (1<<3)

#define line0  B,4      /* PORTB4 */
#define line1  B,3      /* PORTB3 */
#define line2  B,2
#define line3  B,1

typedef struct{
   uint8_t  CRC8;
   uint8_t  Serial[6];
   uint8_t  FamilyCode;
}microlan_rom_t;

typedef union{
      uint8_t Bytes[9];
      struct{
         uint8_t  TempL;
         uint8_t  TempH;
         uint8_t  TempBoundaryH;
         uint8_t  TempBoundaryL;
         uint8_t  ConfigReg;
         uint8_t  Reserved[3];
         uint8_t  CRC8;
      };
}ds18b20_scratch_t;

typedef struct{
   microlan_rom_t    ROM;
   ds18b20_scratch_t Scratchpad;
}microlan_data_t;

#define LineDataInit(line) static microlan_data_t line##Data;\
                           static ds18b20_scratch_t * p##line##Scratchpad = &(line##Data.Scratchpad);\
                           static microlan_rom_t * p##line##ROM = &(line##Data.ROM)

void microlan_lines_init(void);
uint8_t start_converting_temp(uint8_t LinesMask);
uint8_t try_to_get_temp(microlan_data_t * TempData, uint8_t LinesMask);

#define DEBUG

#ifdef DEBUG
void __ml_write1(uint8_t LinesMask);
void __ml_write0(uint8_t LinesMask);
uint8_t __ml_read(uint8_t LinesMask);
uint8_t __ml_reset(uint8_t LinesMask);
#endif

#endif
