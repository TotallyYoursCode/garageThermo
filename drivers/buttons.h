#ifndef _BUTTONS_H
#define _BUTTONS_H

#define BUTTON_UP       BUTTON_0
#define BUTTON_ESC      BUTTON_0

#define BUTTON_DOWN     BUTTON_1
#define BUTTON_OK       BUTTON_1

#define BUTTON_LEFT     BUTTON_2
#define BUTTON_PREV     BUTTON_2

#define BUTTON_RIGHT    BUTTON_3
#define BUTTON_NEXT     BUTTON_3

/* настройки портов */
#define BUTTON_0_PORT     C,4
#define BUTTON_1_PORT     C,5
#define BUTTON_2_PORT     C,6
#define BUTTON_3_PORT     C,7


typedef uint8_t volatile __tiny io_port_t;
typedef struct{
   io_port_t  *PinRegister;
   uint8_t     BitNumber;
   uint8_t     Counter;
   union{
      uint8_t All;
      struct{
         uint8_t  ServiceLock :1,
                  ServiceHold :1,
                  ActionLock  :1,
                  ActionHold  :1;
      };
   }Bit;
}button_data_t;

/* определение переменных для кнопок */
extern button_data_t BUTTON_0, BUTTON_1, BUTTON_2, BUTTON_3;

void buttons_init (void);
void buttons_service(void);
#define button_lock(x)        (x.Bit.ActionLock)
#define button_lock_clr(x)    (x.Bit.ActionLock = 0)
#define button_hold(x)        (x.Bit.ActionHold)
#define button_hold_clr(x)    (x.Bit.ActionHold = 0)
#define button_clr(x)         do{x.Bit.ActionLock = 0; x.Bit.ActionHold = 0;}while(0)

#endif