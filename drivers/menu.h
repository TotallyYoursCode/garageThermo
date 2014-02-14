/* This file is using special IAR extension __flash to allocate menu data in flash memory.
For the best size optimization and SRAM utilization the special compiler option: --string_literals_in_flash
must be added to compiler settings */
#ifndef MENU_H
#define MENU_H


typedef enum{
   MENU_INACTIVE = 0,
   MENU_ACTIVE,
   __LAST_STATE
}menu_state_t;

typedef enum{
   NULL_ACTION = 0,
   CHANGE_TIME,
   CHANGE_DATE,
   CHANGE_DT23_ON,
   CHANGE_DT23_OFF,
   CHANGE_PR0_T1,
   CHANGE_PR1_T1,
   CHANGE_PR2_T1,
   CHANGE_PR0_T2,
   CHANGE_PR1_T2,
   CHANGE_PR2_T2,
   CHANGE_PR0_DT1,
   CHANGE_PR1_DT1,
   CHANGE_PR2_DT1,
   CHANGE_PR0_DT2,
   CHANGE_PR1_DT2,
   CHANGE_PR2_DT2,
   ACTIVATE_PR_0,
   ACTIVATE_PR_1,
   ACTIVATE_PR_2,
   CHANGE_T3,
   CHANGE_DT3,
   ACTIVATE_FAN_AUTO,
   SWITCH_FAN_AUTO,
   SWITCH_FAN_ON,
   SWITCH_FAN_OFF,
   CHANGE_NIGHT_TIME_ON,
   CHANGE_NIGHT_PR_0,
   CHANGE_NIGHT_PR_1,
   CHANGE_NIGHT_PR_2,
   CHANGE_DAY_TIME_ON,
   CHANGE_DAY_PR_0,
   CHANGE_DAY_PR_1,
   CHANGE_DAY_PR_2,
   __LAST_ACTION
}action_t;

typedef enum{
   NULL_PARAM = 0,   
   FAN_STATE,
   PUMP_STATE,
   SHOW_TIME,
   SHOW_DATE,
   SHOW_DT23_ON,
   SHOW_DT23_OFF,
   SHOW_PR0_T1,
   SHOW_PR1_T1,
   SHOW_PR2_T1,
   SHOW_PR0_T2,
   SHOW_PR1_T2,
   SHOW_PR2_T2,
   SHOW_PR0_DT1,
   SHOW_PR1_DT1,
   SHOW_PR2_DT1,
   SHOW_PR0_DT2,
   SHOW_PR1_DT2,
   SHOW_PR2_DT2,
   SHOW_NIGHT_ALL,
   SHOW_NIGHT_TIME_ON,
   SHOW_NIGHT_PROFILE,
   SHOW_DAY_ALL,
   SHOW_DAY_TIME_ON,
   SHOW_DAY_PROFILE,
   SHOW_T3,
   SHOW_DT3,
   __LAST_PARAM
}show_par_t;

menu_state_t menu_get_state(void);;
void menu_set_state(menu_state_t state);
void menu_init(uint8_t * pLcdBuf);
void menu_show_new(void);
void menu_next_item(void);
void menu_prev_item(void);
void menu_next_screen(void);
void menu_prev_screen(void);

#endif
