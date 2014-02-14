/*#######################################################################################################################
//#
//#    ������� ������ � ��������
//#
//# � ������ ���������������� ���������� ������� buttons_init();
//# ������������ ����� � �������� 100�� (������ 10��) - ������� buttons_service();
//# ������ �������� ��������� ������ ������ ������������ � ������� ������� buttons_get();
//#
//####################################################################################################################### */
#include <stdint.h>
#include <ioavr.h>

#include "buttons.h"
#include "ports_avr.h"

/* ��������� ���������� ������ ������� */
#define BUTTON_LOCK_TIME        3              /* ����� ��������� �������� � �������� ���������� (1-10)*/
#define BUTTON_HOLD_TIME        100            /* ����� �������� �������� ������� � �������� ���������� (100 - 256)*/



#define _button_init(port, bit, name)  _SetPinAsInput(port, bit);\
                                       _SetPin(port, bit);\
                                       name##.Counter = 0;\
                                       name##.PinRegister = &(PIN##port);\
                                       name##.BitNumber = bit;\
                                       name##.Bit.All = 0;

#define button_init(button, name)      _button_init(button, name)

#define PIN(x)                         ((*x).PinRegister)
#define BIT(x)                         ((*x).BitNumber)
#define get_button(x)                  ((*PIN(x)) & (1<<BIT(x)))

button_data_t BUTTON_0, BUTTON_1, BUTTON_2, BUTTON_3;


/* ####################################################################################################################### */
/* ��������� ������ �����-������ */
void buttons_init (void){
   button_init(BUTTON_0_PORT,BUTTON_0);
   button_init(BUTTON_1_PORT,BUTTON_1);
   button_init(BUTTON_2_PORT,BUTTON_2);
   button_init(BUTTON_3_PORT,BUTTON_3);
}

void poll_button(button_data_t * data);  
//-----------------------------------------------------------------------------------------------------------------------
//������� ��������� ������� ������ (�������� � ���������� � �������� 100 ��)
//�������� ������� ������������� ��� BTN_SHRT_X ���������� ���������� BtnFlags
//������� ������� ������������� ��� BTN_LONG_X ���������� ���������� BtnFlags
void buttons_service(void){
   poll_button(&BUTTON_0);
   poll_button(&BUTTON_1);
   poll_button(&BUTTON_2);
   poll_button(&BUTTON_3);
}



/* ������� ������ ����� ������ */
void poll_button(button_data_t * data){
   if((*data).Bit.ServiceLock){                              /* 1. ���� ���� ������������� ������� */
      if(!(get_button(data))){                                  /* 2. ���� ��� ��� ������ */
         if(!((*data).Bit.ServiceHold)){                           /* 3. ���� ������, �� �� ����� */
                                                                      /* ����������� ������� ������������ ������� */
            if (++((*data).Counter) == BUTTON_HOLD_TIME){             /* 4. ���� ������� ����� �� �������� ������������� ��������� */
               (*data).Bit.ServiceHold = 1;                              /* ��������� ����� ��������� */
               (*data).Bit.ActionHold = 1;                               /* ��������� ����� �������� �� ��������� */
            }
         }
      } else {                                                  /* 2. ���� ��� �� ������ */
         if((*data).Bit.ServiceHold){                              /* 5. ���� ���� ��������� ���������� */
            (*data).Bit.ServiceHold = 0;                              /* ���������� ��� */
                                                                      /* ����� ����� ���������� ���� �������� �� ���������� ��������� */
         } else {                                                  /* 5. ���� ��������� �� ���� */
            (*data).Bit.ActionLock = 1;                               /* ��������� ����� �������� �� ��������� ���������� */
         }
         (*data).Counter = 0;                                      /* ���������� ������� ������������ ������� */
         (*data).Bit.ServiceLock = 0;                              /* ���������� ���� ��������� ������� */
      }
   } else {                                                  /* 1. ���� ������� �� ���� ������������� */
      if(!(get_button(data))){                                  /* 6. ���� ������ ������ */
                                                                   /* ����������� ������� ������������ ������� */
         if (++(*data).Counter == BUTTON_LOCK_TIME){               /* 7. ���� ������� ����� �� ��������� ������������� ������� */                                                                   
            (*data).Bit.ServiceLock = 1;                              /* ������������� ���� ������� ������ */
                                                                      /* ����� ����� ���������� ���� �������� �� ������� */
         }
      } else {                                                  /* 6. ���� ��� �� ������ */
         (*data).Counter = 0;                                      /* ������� �������� �������, ����� �������� */
      }
   }
}
