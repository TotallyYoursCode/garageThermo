#include <stdint.h>
#include <intrinsics.h>

#include "main.h"                /* заголовок основной программы */
#include "atomic.h"              /* атомарный доступ */
#include "hitachi_lcd_lib.h"     /* библиотека работы с ЖКИ */
#include "microlan.h"            /* библиотека 1-wire */
#include "relay.h"               /* настройка выходов реле */
#include "systimer.h"
#include "cyr_decode.h"
#include "bin2bcd.h"
#include "buttons.h"
//#include "m41t81.h"
#include "menu.h"

typedef  int8_t   temperature_t;
typedef  uint16_t delay_t;
typedef enum{
            Monday    = (1<<0),
            Tuesday   = (1<<1),
            Wednesday = (1<<2),
            Thursday  = (1<<3),
            Friday    = (1<<4),
            Saturday  = (1<<5),
            Sunday    = (1<<6),
            AllWeek   = (1<<7)
}days_in_week_t;

typedef struct{
   uint8_t Minutes;
   uint8_t Hours;
   days_in_week_t DaysInWeek;
}time_t;

temperature_t     Temperature[4];
#define Temp1     Temperature[0]
#define Temp2     Temperature[1]
#define Temp3     Temperature[2]
#define Temp4     Temperature[3]

/* Т2-3on и Т2-3off настройка одна для всех профилей и режимов.
   Температура на разрешение включения вентилятора тоже одна для всех профилей. */
/* common settings */
typedef struct{
   temperature_t  dT23max;
   temperature_t  dT23min;
   temperature_t  T3fanOff;
   temperature_t  T3fanOn;
   delay_t        PumpDelay_s;
}common_settings_t;

/* В настраиваемых профилях настраивается Т1 и dТ1, Т2 и dТ2. */
/* profile settings */
typedef struct{
      temperature_t  T1max;   /* температура выключения обогрева помещения */
      temperature_t  T1min;   /* температура включения обогрева помещения */
      temperature_t  T2max;   /* температура выключения нагревателя (ТЭНа) */
      temperature_t  T2min;   /* температура включения нагревателя (ТЭНа) */
}profile_settings_t;

/* profile type */
typedef enum{
   PROFILE_0 = 0,
   PROFILE_1,
   PROFILE_2,
   __LAST_PROFILE
}profile_t;
#define MAX_PROFILES __LAST_PROFILE

/* Каждому режиму соответствует температурный профиль и время перехода на этот профиль */
/* mode settings */
typedef struct{
   profile_t   Profile;
   time_t      TimeToSwitchOn;
}mode_settings_t;

typedef enum{
   NO_MODE_CHANGE = -1,
   DAY_MODE = 0,
   NIGHT_MODE,
   __LAST_MODE
}mode_t;
#define MAX_MODES __LAST_MODE

/* all settings for current mode */
typedef struct{
   mode_t               Mode;
   mode_settings_t      ModeSettings;
   profile_settings_t   ProfileSettings;
   common_settings_t    CommonSettings;
}all_settings_t;

mode_t get_cur_mode(uint8_t cur_hours, uint8_t cur_minutes, days_in_week_t cur_day);

static __eeprom   common_settings_t    CommonSettings;
static __eeprom   profile_settings_t   ProfileSettings[MAX_PROFILES];
static __eeprom   mode_settings_t      ModeSettings[MAX_MODES];
static            all_settings_t       CurSettings;


/* current profile data initialization */
void thermo_settings_init(void){
   CurSettings.Mode = get_cur_mode(7,15,AllWeek);  /* !!! add definition of current mode by comparing current time with the time to switch on the mode */
   CurSettings.ModeSettings = ModeSettings[CurSettings.Mode];
   CurSettings.ProfileSettings = ProfileSettings[CurSettings.ModeSettings.Profile];
   CurSettings.CommonSettings = CommonSettings;
}

/* изменение параметра настройки профиля */
#define change_profile_value(profile,var,val) \
   if(ProfileSettings[##profile##].##var != val){ \
      ProfileSettings[##profile##].##var  = val; \
      if(CurSettings.ModeSettings.Profile == profile){ \
         __atomic_block(CurSettings.ProfileSettings.##var = val); \
      } \
   }

/* изменение общего параметра настройки */
#define change_common_value(var,val) \
   if(CommonSettings.##var != val){ \
      CommonSettings.##var  = val; \
      __atomic_block(CurSettings.CommonSettings.##var = val); \
   }

#define set_profile(profile) \
   if(CurSettings.ModeSettings.Profile != profile){ \
      __atomic_block(CurSettings.ModeSettings.Profile = profile;); \
      __atomic_block(CurSettings.ProfileSettings = ProfileSettings[profile];); \
   }

/* чтение параметра из памяти */
#define get_profile_value(val)               (CurSettings.ProfileSettings.##val)
#define get_common_value(val)                (CurSettings.CommonSettings.##val)

         

/* флаги системы */
struct{
  uint8_t   Bit0                 :1, /*  */
            ConversionStarted    :1, /* запущено измерение температуры */
            PumpDelay            :1, /* режим ожидания перед выключением насоса */
            PumpAnimateState     :1, /* номер кадра анимации насоса */
            HeaterAnimateState   :1, /* номер кадра анимации нагревателя */
            FanAnimateState      :1, /* номер кадра анимации вентилятора */
            Bit6:1,
            Bit7:1;
}static Flag;

#define set_flag(flag)              __atomic_block(Flag.##flag = 1)
#define clr_flag(flag)              __atomic_block(Flag.##flag = 0)
#define set_value_flag(flag,value)  __atomic_block(Flag.##flag = value)
#define toggle_flag(flag)           __atomic_block(Flag.##flag ^= 1)
#define check_flag(flag)            (Flag.##flag)

enum{
   FAN_ICON = 0,
   PUMP_ICON,
   HEATER_ICON,
   T1_SYMBOL,
   T2_SYMBOL,
   T3_SYMBOL
};

__flash uint8_t _T1_symbol[]={0x1C,0x08,0x09,0x0B,0x09,0x01,0x01,0x00};
__flash uint8_t _T2_symbol[]={0x1C,0x08,0x0B,0x09,0x0B,0x02,0x03,0x00};
__flash uint8_t _T3_symbol[]={0x1C,0x08,0x0B,0x09,0x0B,0x01,0x03,0x00};
__flash uint8_t _Heater_0[] ={0x02,0x04,0x0C,0x0A,0x09,0x15,0x0E,0x00};
__flash uint8_t _Heater_1[] ={0x08,0x04,0x06,0x05,0x09,0x15,0x0E,0x00};
__flash uint8_t _Pump_0[]   ={0x1C,0x08,0x08,0x1C,0x17,0x14,0x1C,0x00};
__flash uint8_t _Pump_1[]   ={0x00,0x1C,0x08,0x1C,0x1F,0x14,0x1C,0x00};
__flash uint8_t _Fan_0[]    ={0x00,0x02,0x14,0x0A,0x05,0x08,0x00,0x00};
__flash uint8_t _Fan_1[]    ={0x00,0x08,0x0B,0x04,0x1A,0x02,0x00,0x00};
__flash uint8_t _Pump_v2_0[]={0x0E,0x04,0x04,0x0E,0x0A,0x1B,0x0E,0x00};
__flash uint8_t _Pump_v2_1[]={0x00,0x0E,0x04,0x0E,0x0E,0x1B,0x0E,0x00};

#define ICON_ANIMATE 0


typedef enum{
   SWITCHED_OFF = 0,
   COLD_SYS_HEATING,
   HOT_SYS_CIRCULATION,
   COOLING_SYSTEM
}system_state_t;

system_state_t sysstate_get(void);
void sysstate_set(system_state_t state);


uint8_t LcdBuffer[32];  /* буфер индикатора */

void system_SM(void);              /* реализация машины состояний алгоритма терморегуляции */
void system_SM_init(void);         /* инициализация машины состояний алгоритма терморегуляции */


#define THERMO_FAILURE_DELAY_s  10

__noreturn __C_task main( void ) {
   microlan_data_t MicrolanData[4];
   uint8_t LinesWithSensors,LinesWithoutErrors;
   
   system_SM_init();             /* инициализация терморегуляции */
   hitachi_lcd_init(LcdBuffer);  /* инициализация драйвера дисплея */   
   microlan_lines_init();        /* инициализация линий MicroLAN */
   relay_lines_init();
   buttons_init();
   
   change_profile_value(PROFILE_0,T1max, 10);  // если профиль текущий и EEPROM меняется, то RAM тоже меняется
   change_profile_value(PROFILE_0,T1min, 5);
   change_profile_value(PROFILE_0,T2max, 80);
   change_profile_value(PROFILE_0,T2min, 70);
   change_common_value(dT23min, 5);               // если EEPROM меняется, то RAM тоже меняется 
   change_common_value(dT23max, 10);   
   change_common_value(T3fanOff, 50);
   change_common_value(T3fanOn, 60);
   change_common_value(PumpDelay_s, 60);
   set_profile(PROFILE_0);                      // если профиль не текущий, RAM инициализируется из EEPROM
                                                // на случай если профиль текущий совпал, а EEPROM  не менялась
   thermo_settings_init();                      /* инициализация данных RAM из EEPROM последнего использованного профиля */
   
   
   systimer_init();              /* systimer initialization */
   msec_timer_event_config(THERMOMETERS_POLL_EVENT, THERMOMETERS_POLL_PERIOD_ms, NULL);   /* therm poll event */
   msec_timer_event_config(LCD_SERVICE_EVENT, LCD_SERVICE_PERIOD_ms, NULL);               /* lcd service event */
   sec_timer_event_config(THERMOMETERS_FAILURE_EVENT, THERMO_FAILURE_DELAY_s, NULL);    /* sensors failure event */
   msec_timer_event_config(PUMP_ANIMATE_EVENT, 410, NULL);                             /* pump animate event */
   msec_timer_event_config(HEATER_ANIMATE_EVENT, 390, NULL);                           /* heater animate event */
   msec_timer_event_config(FAN_ANIMATE_EVENT, 400, NULL);                              /* fan animate event */
   msec_timer_event_config(BUTTONS_SERVICE_EVENT, 10, NULL);                              /*  */
   
     
   __enable_interrupt();         /* глобальное разрешение прерываний */

   /*
   //hitachi_lcd_cgram_symbol(_T1_symbol, T1_SYMBOL);
   //hitachi_lcd_cgram_symbol(_T2_symbol, T2_SYMBOL);
   //hitachi_lcd_cgram_symbol(_T3_symbol, T3_SYMBOL);
   hitachi_lcd_cgram_symbol(_Fan_1, FAN_ICON);
   hitachi_lcd_cgram_symbol(_Pump_1, PUMP_ICON);
   hitachi_lcd_cgram_symbol(_Heater_1, HEATER_ICON);
   LcdBuffer[6] = FAN_ICON;
   LcdBuffer[7] = PUMP_ICON;
   LcdBuffer[22] = HEATER_ICON;
   */
   
   menu_init(LcdBuffer);   
   
   put_str(&LcdBuffer[0], "Привет!");
   for(;;){

      if(msec_timer_event_get(THERMOMETERS_POLL_EVENT)){
         msec_timer_event_clr(THERMOMETERS_POLL_EVENT);
         
         if(check_flag(ConversionStarted)){
            LinesWithoutErrors = try_to_get_temp(MicrolanData,LinesWithSensors); /* попытка считать скретчпад термометров */
            if(LinesWithoutErrors == (T1ch|T2ch|T3ch)){                          /* все данные без ошибок */
               sec_timer_event_clr(THERMOMETERS_FAILURE_EVENT);                  /* сброс таймера отказа термометров */               
               Temp1 = ((MicrolanData[0].Scratchpad.TempL>>4) | ((MicrolanData[0].Scratchpad.TempH<<4)&0xF0));
               Temp2 = ((MicrolanData[1].Scratchpad.TempL>>4) | ((MicrolanData[1].Scratchpad.TempH<<4)&0xF0));
               Temp3 = ((MicrolanData[2].Scratchpad.TempL>>4) | ((MicrolanData[2].Scratchpad.TempH<<4)&0xF0));
               system_SM();
            }
         }
         
         LinesWithSensors = start_converting_temp(T1ch|T2ch|T3ch); /* запуск конвертации температуры */
         if(LinesWithSensors == (T1ch|T2ch|T3ch)){
            set_flag(ConversionStarted);                           /* все запрошенные термометры начали конвертацию */
         } else {
            clr_flag(ConversionStarted);
         }
      }
            
      if(button_lock(BUTTON_OK)){
         button_lock_clr(BUTTON_OK);
         if(menu_get_state() == MENU_ACTIVE){
            menu_next_screen();
         } else {
            menu_set_state(MENU_ACTIVE);
         }
      }
      
      if(button_lock(BUTTON_ESC)){
         button_lock_clr(BUTTON_ESC);
         menu_prev_screen();
      }
      
      if(button_lock(BUTTON_NEXT)){
         button_lock_clr(BUTTON_NEXT);
         menu_next_item();
      }
      
      if(button_lock(BUTTON_PREV)){
         button_lock_clr(BUTTON_PREV);
         menu_prev_item();
      }

            
      
      if(sec_timer_event_get(THERMOMETERS_FAILURE_EVENT)){
         sec_timer_event_dis(THERMOMETERS_FAILURE_EVENT);
         switch_off(HEATER);
         switch_on(PUMP);
         switch_off(FAN);
         put_str(&LcdBuffer[23], "  Alarm! ");
         sysstate_set(SWITCHED_OFF);
      }
            
     
      if(msec_timer_event_get(BUTTONS_SERVICE_EVENT)){ /* опрос кнопок */
         msec_timer_event_clr(BUTTONS_SERVICE_EVENT); 
         buttons_service();
      }
      
      if(msec_timer_event_get(LCD_SERVICE_EVENT)){ /* обслуживание дисплея */
         msec_timer_event_clr(LCD_SERVICE_EVENT); 
         hitachi_lcd_service();
      }

      /* Icons animation */
#if ICON_ANIMATE
      if(msec_timer_event_get(PUMP_ANIMATE_EVENT)){
         msec_timer_event_clr(PUMP_ANIMATE_EVENT);
         if(check_flag(PumpAnimateState))
            hitachi_lcd_cgram_symbol(_Pump_v2_0, PUMP_SYMBOL);
         else
            hitachi_lcd_cgram_symbol(_Pump_v2_1, PUMP_SYMBOL);
         toggle_flag(PumpAnimateState);
      }
      
      if(msec_timer_event_get(HEATER_ANIMATE_EVENT)){
         msec_timer_event_clr(HEATER_ANIMATE_EVENT);
         if(check_flag(HeaterAnimateState))
            hitachi_lcd_cgram_symbol(_Heater_0, HEATER_SYMBOL);
         else
            hitachi_lcd_cgram_symbol(_Heater_1, HEATER_SYMBOL);
         toggle_flag(HeaterAnimateState);
      }
      
      if(msec_timer_event_get(FAN_ANIMATE_EVENT)){
         msec_timer_event_clr(FAN_ANIMATE_EVENT);
         if(check_flag(FanAnimateState))
            hitachi_lcd_cgram_symbol(_Fan_0, FAN_SYMBOL);
         else
            hitachi_lcd_cgram_symbol(_Fan_1, FAN_SYMBOL);
         toggle_flag(FanAnimateState);
      }
#endif
   }
}

mode_t get_cur_mode(uint8_t cur_hours, uint8_t cur_minutes, days_in_week_t cur_day){
   uint8_t i;
   for(i = 0; i < MAX_MODES; i++){
      time_t ModeTime;
      ModeTime =  ModeSettings[(mode_t)i].TimeToSwitchOn;
      if((ModeTime.Hours == cur_hours) && (ModeTime.Minutes == cur_minutes) && ((ModeTime.DaysInWeek & AllWeek) || (ModeTime.DaysInWeek & cur_day)))
         return (mode_t)i;
   }
   return NO_MODE_CHANGE;
}


/* ========================================= */
/* Машина состояний алгоритма терморегуляции */
void SwitchedOff(void);
void ColdSysHeating(void);
void HotSysCirculation(void);
void CoolingSystem(void);

void (*system_state_handler[])()={
   SwitchedOff,
   ColdSysHeating,
   HotSysCirculation,
   CoolingSystem
};

static system_state_t CurrSystemState;

void system_SM(void){
   system_state_handler[CurrSystemState]();
}

system_state_t sysstate_get(void){
   return CurrSystemState;
}

void sysstate_set(system_state_t state){
   CurrSystemState = state;
}

void system_SM_init(){
   CurrSystemState = SWITCHED_OFF;
   
}
 
void SwitchedOff(void){
   switch_off(HEATER);
   switch_off(PUMP);
   switch_off(FAN);
   if(Temp1 <= get_profile_value(T1min)){
      CurrSystemState = COLD_SYS_HEATING;
   }
}

void ColdSysHeating(void){
   /* При снижении температуры t1 включается нагрев и насос по разности температур.
      При снижении t2 включается нагрев, а насос включится только по разности температур. */
   switch_on(HEATER);                     
   if((Temp2-Temp3) >= get_common_value(dT23max)){  
      switch_on(PUMP);
   }
   if((Temp2-Temp3) <= get_common_value(dT23min)){
      switch_off(PUMP);
   }
   /* *********** */
   
   /* Нагрев тена контролируется только по t2 минус дельта (настраиваемый). */
   if(Temp2 >= get_profile_value(T2max)){
      CurrSystemState = HOT_SYS_CIRCULATION;
   }
   /* *********** */
      
   /* Разрешить работу вентилятора по достижении установленной температуры по t3 (в меню настроек) и только в момент работы процеса нагрева  по t1 плюс дельта */
   if(Temp3 >= get_common_value(T3fanOn)){
      switch_on(FAN);
   }
   if(Temp3 <= get_common_value(T3fanOff)){
      switch_off(FAN);
   }
   /* *********** */
   
   if(Temp1 >= get_profile_value(T1max)){
      CurrSystemState = COOLING_SYSTEM;
   }
}

void HotSysCirculation(void){
   /* При достижении t2 насос включается на постоянку (на разность температур уже не смотрим). */
   switch_on(PUMP);
   /* *********** */
   
   switch_off(HEATER);
   
   /* Разрешить работу вентилятора по достижении установленной температуры по t3 (в меню настроек) и только в момент работы процеса нагрева  по t1 плюс дельта */
   if(Temp3 > get_common_value(T3fanOn)){
      switch_on(FAN);
   }
   if(Temp3 < get_common_value(T3fanOff)){
      switch_off(FAN);
   }
   /* *********** */
   
   /* Насос на постоянке работает до температуры t2 минус дельта. */
   if(Temp2 <= get_profile_value(T2min)){
      CurrSystemState = COLD_SYS_HEATING;
   }
   /* *********** */
   
   if(Temp1 >= get_profile_value(T1max)){
      CurrSystemState = COOLING_SYSTEM;
   }
   
}
      
void CoolingSystem(void){
   /* По достижении t1 плюс дельта, нагрев отключается, насос отрабатывает по таймеру. Дальше ждем t1. */
   switch_off(HEATER);
   //switch_on(PUMP);   // не меняем состояние насоса и вентилятора.
   //switch_on(FAN);
   if(check_flag(PumpDelay)){
      if(sec_timer_event_get(PUMP_DELAY_EVENT)){
         sec_timer_event_clr(PUMP_DELAY_EVENT);
         sec_timer_event_dis(PUMP_DELAY_EVENT);
         clr_flag(PumpDelay);
         CurrSystemState = SWITCHED_OFF;
      }
   } else {      
      set_flag(PumpDelay);
      sec_timer_event_config(PUMP_DELAY_EVENT, get_common_value(PumpDelay_s), NULL); /* pump delay event */
   }
}
/* ======================================= */


