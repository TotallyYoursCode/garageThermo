#include <stdint.h>
#include <intrinsics.h>

#include "main.h"                /* ��������� �������� ��������� */
#include "atomic.h"              /* ��������� ������ */
#include "hitachi_lcd_lib.h"     /* ���������� ������ � ��� */
#include "microlan.h"            /* ���������� 1-wire */
#include "relay.h"               /* ��������� ������� ���� */
#include "systimer.h"
#include "cyr_decode.h"
#include "bin2bcd.h"
#include "buttons.h"
//#include "m41t81.h"
#include "menu.h"

typedef  int8_t   temperature_t;
typedef  uint16_t delay_t;
typedef struct{
   uint8_t  Monday:1,
            Tuesday:1,
            Wednesday:1,
            Thursday:1,
            Friday:1,
            Saturday:1,
            Sunday:1,
            AllWeek:1;
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

/* �2-3on � �2-3off ��������� ���� ��� ���� �������� � �������.
   ����������� �� ���������� ��������� ����������� ���� ���� ��� ���� ��������. */
/* common settings */
typedef struct{
   temperature_t  dT23max;
   temperature_t  dT23min;
   temperature_t  T3fanOff;
   temperature_t  T3fanOn;
   delay_t        PumpDelay_s;
}common_settings_t;

/* � ������������� �������� ������������� �1 � d�1, �2 � d�2. */
/* profile settings */
typedef struct{
      temperature_t  Temp1max;   /* ����������� ���������� �������� ��������� */
      temperature_t  Temp1min;   /* ����������� ��������� �������� ��������� */
      temperature_t  Temp2max;   /* ����������� ���������� ����������� (����) */
      temperature_t  Temp2min;   /* ����������� ��������� ����������� (����) */
}profile_settings_t;

/* profile type */
typedef enum{
   PROFILE_0 = 0,
   PROFILE_1,
   PROFILE_2,
   __LAST_PROFILE
}profile_t;
#define MAX_PROFILES __LAST_PROFILE

/* ������� ������ ������������� ������������� ������� � ����� �������� �� ���� ������� */
/* mode settings */
typedef struct{
   profile_t   Profile;
   time_t      TimeToSwitchOn;
}mode_settings_t;

typedef enum{
   DAY_MODE = 0,
   NIGHT_MODE,
   __LAST_MODE
}mode_t;
#define MAX_MODES __LAST_MODE

/* all settings for current mode */
typedef struct{
   profile_settings_t   CurProfileSettings;
   common_settings_t    CommonSettings;
   profile_t            CurProfile;
   mode_settings_t      CurMode;
}all_settings_t;

static __eeprom   common_settings_t    CommonSettings;
static __eeprom   profile_settings_t   ProfileSettings[MAX_PROFILES];
static __eeprom   mode_settings_t      ModeSettings[MAX_MODES];
static            all_settings_t       FastAscessSettings;


/* current profile data initialization */
void profile_init(void){
   //FastAscessSettings.CurProfile = ModeSettings[CurMode.Profile;
   //FastAscessSettings.CurProfileSettings = ProfileSettings[FastAscessSettings.CurrentProfile];
   //FastAscessSettings.Common = CommonSettings;
}

/* ��������� ��������� ��������� ������� */
#define save_profile_value(profile,var,val) \
   if(ProfileSettings[##profile##].##var != val){ \
      ProfileSettings[##profile##].##var  = val; \
      if(FastAscessSettings.CurrentProfile == profile){ \
         __atomic_block(FastAscessSettings.Profile.##var = val); \
      } \
   }

/* ��������� ������ ��������� ��������� */
#define save_common_value(var,val) \
   if(CommonSettings.##var != val){ \
      CommonSettings.##var  = val; \
      __atomic_block(FastAscessSettings.Common.##var = val); \
   }

#define set_profile(profile) \
   if(LastProfile != profile){ \
      LastProfile  = profile; \
      __atomic_block(profile_init()); \
   }
         
/* ������ ��������� �� ������ */
#define get_profile_value(val)               (FastAscessSettings.Profile.##val)
#define get_common_value(val)                (FastAscessSettings.Common.##val)
         

/* ����� ������� */
struct{
  uint8_t   Bit0                 :1, /*  */
            ConversionStarted    :1, /* �������� ��������� ����������� */
            PumpDelay            :1, /* ����� �������� ����� ����������� ������ */
            PumpAnimateState     :1, /* ����� ����� �������� ������ */
            HeaterAnimateState   :1, /* ����� ����� �������� ����������� */
            FanAnimateState      :1, /* ����� ����� �������� ����������� */
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


uint8_t LcdBuffer[32];  /* ����� ���������� */

void system_SM(void);              /* ���������� ������ ��������� ��������� �������������� */
void system_SM_init(void);         /* ������������� ������ ��������� ��������� �������������� */


#define THERMO_FAILURE_DELAY_s  10

__noreturn __C_task main( void ) {
   microlan_data_t MicrolanData[4];
   uint8_t LinesWithSensors,LinesWithoutErrors;
   
   system_SM_init();             /* ������������� �������������� */
   hitachi_lcd_init(LcdBuffer);  /* ������������� �������� ������� */   
   microlan_lines_init();        /* ������������� ����� MicroLAN */
   relay_lines_init();
   buttons_init();
   
   save_profile_value(PROFILE_0,Temp1max, 10);  // ���� ������� ������� � EEPROM ��������, �� RAM ���� ��������
   save_profile_value(PROFILE_0,Temp1min, 5);
   save_profile_value(PROFILE_0,Temp2max, 80);
   save_profile_value(PROFILE_0,Temp2min, 70);
   save_common_value(dT23min, 5);               // ���� EEPROM ��������, �� RAM ���� �������� 
   save_common_value(dT23max, 10);   
   save_common_value(T3fanOff, 50);
   save_common_value(T3fanOn, 60);
   save_common_value(PumpDelay_s, 60);
   set_profile(PROFILE_0);                      // ���� ������� �� �������, RAM ���������������� �� EEPROM
                                                // �� ������ ���� ������� ������� ������, � EEPROM  �� ��������
   profile_init();                              /* ������������� ������ RAM �� EEPROM ���������� ��������������� ������� */
   
   
   systimer_init();              /* systimer initialization */
   msec_timer_event_config(THERMOMETERS_POLL_EVENT, THERMOMETERS_POLL_PERIOD_ms, NULL);   /* therm poll event */
   msec_timer_event_config(LCD_SERVICE_EVENT, LCD_SERVICE_PERIOD_ms, NULL);               /* lcd service event */
   sec_timer_event_config(THERMOMETERS_FAILURE_EVENT, THERMO_FAILURE_DELAY_s, NULL);    /* sensors failure event */
   msec_timer_event_config(PUMP_ANIMATE_EVENT, 410, NULL);                             /* pump animate event */
   msec_timer_event_config(HEATER_ANIMATE_EVENT, 390, NULL);                           /* heater animate event */
   msec_timer_event_config(FAN_ANIMATE_EVENT, 400, NULL);                              /* fan animate event */
   msec_timer_event_config(BUTTONS_SERVICE_EVENT, 10, NULL);                              /*  */
   
     
   __enable_interrupt();         /* ���������� ���������� ���������� */

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
   
   put_str(&LcdBuffer[0], "������!");
   for(;;){

      if(msec_timer_event_get(THERMOMETERS_POLL_EVENT)){
         msec_timer_event_clr(THERMOMETERS_POLL_EVENT);
         
         if(check_flag(ConversionStarted)){
            LinesWithoutErrors = try_to_get_temp(MicrolanData,LinesWithSensors); /* ������� ������� ��������� ����������� */
            if(LinesWithoutErrors == (T1ch|T2ch|T3ch)){                          /* ��� ������ ��� ������ */
               sec_timer_event_clr(THERMOMETERS_FAILURE_EVENT);                  /* ����� ������� ������ ����������� */               
               Temp1 = ((MicrolanData[0].Scratchpad.TempL>>4) | ((MicrolanData[0].Scratchpad.TempH<<4)&0xF0));
               Temp2 = ((MicrolanData[1].Scratchpad.TempL>>4) | ((MicrolanData[1].Scratchpad.TempH<<4)&0xF0));
               Temp3 = ((MicrolanData[2].Scratchpad.TempL>>4) | ((MicrolanData[2].Scratchpad.TempH<<4)&0xF0));
               system_SM();
            }
         }
         
         LinesWithSensors = start_converting_temp(T1ch|T2ch|T3ch); /* ������ ����������� ����������� */
         if(LinesWithSensors == (T1ch|T2ch|T3ch)){
            set_flag(ConversionStarted);                           /* ��� ����������� ���������� ������ ����������� */
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
            
     
      if(msec_timer_event_get(BUTTONS_SERVICE_EVENT)){ /* ����� ������ */
         msec_timer_event_clr(BUTTONS_SERVICE_EVENT); 
         buttons_service();
      }
      
      if(msec_timer_event_get(LCD_SERVICE_EVENT)){ /* ������������ ������� */
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

/* ========================================= */
/* ������ ��������� ��������� �������������� */
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
   if(Temp1 <= get_profile_value(Temp1min)){
      CurrSystemState = COLD_SYS_HEATING;
   }
}

void ColdSysHeating(void){
   /* ��� �������� ����������� t1 ���������� ������ � ����� �� �������� ����������.
      ��� �������� t2 ���������� ������, � ����� ��������� ������ �� �������� ����������. */
   switch_on(HEATER);                     
   if((Temp2-Temp3) >= get_common_value(dT23max)){  
      switch_on(PUMP);
   }
   if((Temp2-Temp3) <= get_common_value(dT23min)){
      switch_off(PUMP);
   }
   /* *********** */
   
   /* ������ ���� �������������� ������ �� t2 ����� ������ (�������������). */
   if(Temp2 >= get_profile_value(Temp2max)){
      CurrSystemState = HOT_SYS_CIRCULATION;
   }
   /* *********** */
      
   /* ��������� ������ ����������� �� ���������� ������������� ����������� �� t3 (� ���� ��������) � ������ � ������ ������ ������� �������  �� t1 ���� ������ */
   if(Temp3 >= get_common_value(T3fanOn)){
      switch_on(FAN);
   }
   if(Temp3 <= get_common_value(T3fanOff)){
      switch_off(FAN);
   }
   /* *********** */
   
   if(Temp1 >= get_profile_value(Temp1max)){
      CurrSystemState = COOLING_SYSTEM;
   }
}

void HotSysCirculation(void){
   /* ��� ���������� t2 ����� ���������� �� ��������� (�� �������� ���������� ��� �� �������). */
   switch_on(PUMP);
   /* *********** */
   
   switch_off(HEATER);
   
   /* ��������� ������ ����������� �� ���������� ������������� ����������� �� t3 (� ���� ��������) � ������ � ������ ������ ������� �������  �� t1 ���� ������ */
   if(Temp3 > get_common_value(T3fanOn)){
      switch_on(FAN);
   }
   if(Temp3 < get_common_value(T3fanOff)){
      switch_off(FAN);
   }
   /* *********** */
   
   /* ����� �� ��������� �������� �� ����������� t2 ����� ������. */
   if(Temp2 <= get_profile_value(Temp2min)){
      CurrSystemState = COLD_SYS_HEATING;
   }
   /* *********** */
   
   if(Temp1 >= get_profile_value(Temp1max)){
      CurrSystemState = COOLING_SYSTEM;
   }
   
}
      
void CoolingSystem(void){
   /* �� ���������� t1 ���� ������, ������ �����������, ����� ������������ �� �������. ������ ���� t1. */
   switch_off(HEATER);
   //switch_on(PUMP);   // �� ������ ��������� ������ � �����������.
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


