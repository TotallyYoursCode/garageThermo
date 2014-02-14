/*#######################################################################################################################
//#
//#    ФУНКЦИИ РАБОТЫ С КНОПКАМИ
//#
//# в начале инициализировать библиотеку вызовом buttons_init();
//# организовать вызов с частотой 100Гц (период 10мс) - функцию buttons_service();
//# чтение значения состояния флагов кнопок производится с помощью функции buttons_get();
//#
//####################################################################################################################### */
#include <stdint.h>
#include <ioavr.h>

#include "buttons.h"
#include "ports_avr.h"

/* настройка параметров работы функций */
#define BUTTON_LOCK_TIME        3              /* время обработки дребезга в десятках милисекунд (1-10)*/
#define BUTTON_HOLD_TIME        100            /* время фиксации длинного нажатия в десятках милисекунд (100 - 256)*/



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
/* настройка портов ввода-вывода */
void buttons_init (void){
   button_init(BUTTON_0_PORT,BUTTON_0);
   button_init(BUTTON_1_PORT,BUTTON_1);
   button_init(BUTTON_2_PORT,BUTTON_2);
   button_init(BUTTON_3_PORT,BUTTON_3);
}

void poll_button(button_data_t * data);  
//-----------------------------------------------------------------------------------------------------------------------
//ФУНКЦИЯ ОБРАБОТКИ НАЖАТИЙ КЛАВИШ (вызывать в прерывании с частотой 100 Гц)
//короткое нажатие устанавливает бит BTN_SHRT_X глобальной переменной BtnFlags
//длинное нажатие устанавливает бит BTN_LONG_X глобальной переменной BtnFlags
void buttons_service(void){
   poll_button(&BUTTON_0);
   poll_button(&BUTTON_1);
   poll_button(&BUTTON_2);
   poll_button(&BUTTON_3);
}



/* функция опроса одной кнопки */
void poll_button(button_data_t * data){
   if((*data).Bit.ServiceLock){                              /* 1. Если было зафиксировано нажатие */
      if(!(get_button(data))){                                  /* 2. Если все еще нажата */
         if(!((*data).Bit.ServiceHold)){                           /* 3. Если нажата, но не долго */
                                                                      /* увеличиваем счетчик длительности нажатия */
            if (++((*data).Counter) == BUTTON_HOLD_TIME){             /* 4. Если счетчик дошел до значения определяющего удержание */
               (*data).Bit.ServiceHold = 1;                              /* устанавка флага удержания */
               (*data).Bit.ActionHold = 1;                               /* установка флага действия по удержанию */
            }
         }
      } else {                                                  /* 2. Если уже не нажата */
         if((*data).Bit.ServiceHold){                              /* 5. Если флаг удержания установлен */
            (*data).Bit.ServiceHold = 0;                              /* сбрасываем его */
                                                                      /* здесь можно установить флаг действия по отпусканию удержания */
         } else {                                                  /* 5. Если удержания не было */
            (*data).Bit.ActionLock = 1;                               /* установка флага действия по короткому отпусканию */
         }
         (*data).Counter = 0;                                      /* сбрасываем счетчик длительности нажатия */
         (*data).Bit.ServiceLock = 0;                              /* сбрасываем флаг короткого нажатия */
      }
   } else {                                                  /* 1. Если нажатие не было зафиксировано */
      if(!(get_button(data))){                                  /* 6. Если кнопка нажата */
                                                                   /* увеличиваем счетчик длительности нажатия */
         if (++(*data).Counter == BUTTON_LOCK_TIME){               /* 7. Если счетчик дошел до значением определяющего нажатие */                                                                   
            (*data).Bit.ServiceLock = 1;                              /* устанавливаем флаг нажатия кнопки */
                                                                      /* здесь можно установить флаг действия по нажатию */
         }
      } else {                                                  /* 6. Если уже не нажата */
         (*data).Counter = 0;                                      /* слишком короткое нажатие, сброс счетчика */
      }
   }
}
