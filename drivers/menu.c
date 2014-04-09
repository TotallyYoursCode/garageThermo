#include <stdint.h>
#include "menu.h"
#include "main.h"
#include "bin2bcd.h"

#include <stdint.h>

#define com1(a,b)             a##b
#define com(a,b)              com1(a,b)

#define LCD_BUFFER_P          (pMenuBuffer)
#define LCD_HEADER_P          (pMenuBuffer)
#define LCD_ITEM_P            (pMenuBuffer + 16)

// Пункт:
typedef struct {
   const uint8_t  __flash *   pText;          /* Текст Пункта */
   const action_t             Action;         /* Код действия, выполняемого при нажатии ОК на этом Пункте меню */
   const show_par_t           Parameter;
   const void     __flash *   pChildScreen;   /* Указатель на структуру следующего Экрана меню */
} item_t;


// Экран:
typedef struct {
   const uint8_t  __flash *            pHeader;        /* Заголовок Экрана */
   const void     __flash *            pParentScreen;  /* Указатель на структуру предыдущего Экрана меню */
   const uint8_t                       ParentItem;     /* Номер Пункта, вызвавшего этот Экран */
   const uint8_t                       ItemsOnScreen;  /* Кол-во Пунктов меню на Экране */
   const item_t __flash * __flash *    ppItemsArray;   /* Указатель на массив структур */
} screen_t;

/* Определение нового пункта меню */
#define NEW_ITEM(Name, Text, PChildScreen, Parameter, Action) \
extern __flash screen_t PChildScreen; \
static __flash item_t Name = { (uint8_t __flash *)&(Text), Action, Parameter, (void __flash *)&(PChildScreen)}


/* Определение нового эрана меню */
#define NEW_SCR(Name, Header, ParentScr, ParentItem) \
extern __flash screen_t ParentScr; \
static __flash screen_t Name = { \
            (uint8_t __flash *)&(Header), \
            (void __flash *) &(ParentScr), \
            ParentItem, \
            sizeof(com(Name,items))/sizeof(com(com(Name,items),[0])), \
            (const item_t __flash * __flash *)(com(Name,items)) \
}

#define NEW_MINOR_SCR(Header, ParentScr, ParentItem)  NEW_SCR(ParentScr##_##ParentItem, Header, ParentScr, ParentItem)


/* Определение пустого указателя на экран */
#define NULL_SCR Null_Screen
static __flash screen_t Null_Screen = { (uint8_t __flash *)0,
                                 (void __flash *)0,
                                 0,
                                 0,
                                 (const item_t __flash * __flash *)0
};


#define NULL_SCR_P Null_Screen_Pointer
static __flash screen_t __flash * Null_Screen_Pointer = &Null_Screen;

static screen_t __flash * pCurrScreen;             /* текущий экран и номер активного пункта меню */
static uint8_t    CurrItem;                        /* текущий пункт меню */
static uint8_t * pMenuBuffer;                      /* указатель на буфер дисплея */
static menu_state_t MenuState;


#define DEFINES 1
#if DEFINES
#define ITEM_TEXT(ITEM_NUMBER)   (uint8_t __flash *)(((item_t __flash *)((pCurrScreen->ppItemsArray)[ITEM_NUMBER]))->pText)
//(uint8_t __flash *)((*((pCurrScreen->pItemsArray)[ITEM_NUMBER])).pText)
                                 // Получение текста любого пункта меню
#define ITEM_ACTION(ITEM_NUMBER) (action_t)((pCurrScreen->ppItemsArray)[ITEM_NUMBER])->Action
                                 // Получение выполняемого действия любого пункта меню
#define NEXT_SCR_P()             (screen_t __flash *)((pCurrScreen->ppItemsArray)[CurrItem])->pChildScreen
                                 // Получение указателя на следующий экран
#define PREV_SCR_P()             (screen_t __flash *)pCurrScreen->pParentScreen
                                 // Получение указателя на предыдущий экран
#define CURR_HEADER()            (uint8_t __flash *)(pCurrScreen->pHeader)
                                 // Получение заголовка текущего экрана
#define ITEMS_ON_SCREEN()        (uint8_t)pCurrScreen->ItemsOnScreen
                                 // Получение количества пунктов текущего экрана
#define PARENT_ITEM()            (uint8_t)pCurrScreen->ParentItem
                                 /* Получение номер пункта, вызвавшего текущее меню */
#define ITEM_PARAMETER(ITEM_NUMBER) (show_par_t)((pCurrScreen->ppItemsArray)[ITEM_NUMBER])->Parameter

#else
uint8_t __flash * ITEM_TEXT(uint8_t ITEM_NUMBER){
   return (uint8_t __flash *)((((item_t __flash *)(pCurrScreen->ppItemsArray))+ITEM_NUMBER)->pText);
}

action_t ITEM_ACTION(uint8_t ITEM_NUMBER){
   return (action_t)((pCurrScreen->ppItemsArray)[ITEM_NUMBER])->Action;
}

screen_t __flash * NEXT_SCR_P(void){
   return (screen_t __flash *)((pCurrScreen->ppItemsArray)[CurrItem])->pChildScreen;
}

screen_t __flash * PREV_SCR_P(void){
   return (screen_t __flash *)pCurrScreen->pParentScreen;
}

uint8_t __flash * CURR_HEADER(void){
   return (uint8_t __flash *)pCurrScreen->pHeader;
}

uint8_t ITEMS_ON_SCREEN(void){
   return (uint8_t)pCurrScreen->ItemsOnScreen;
}

uint8_t PARENT_ITEM(void){
   return (uint8_t)pCurrScreen->ParentItem;
}
#endif

#define INIT_SCREEN_P &screen0
/* первый уровень */
   /* второй уровень */
      /* третий уровень */
NEW_ITEM(item0scr0, "Режимы", screen0_0, NULL_PARAM, NULL_ACTION);
NEW_ITEM(item1scr0, "Профили", screen0_1, NULL_PARAM, NULL_ACTION);
NEW_ITEM(item2scr0, "Календарь", screen0_2, NULL_PARAM, NULL_ACTION);
__flash item_t __flash * screen0items[] = {&item0scr0,&item1scr0,&item2scr0};
NEW_SCR(screen0, "Меню", NULL_SCR, 0);
   
   // подменю Меню\Режимы
   NEW_ITEM(item0scr0_0, "Терморегуляция", screen0_0_0, NULL_PARAM, NULL_ACTION);
   NEW_ITEM(item1scr0_0, "Вентилятор:", screen0_0_1, SHOW_FAN_STATE, NULL_ACTION);
   NEW_ITEM(item2scr0_0, "Насос:     ", screen0_0_2, SHOW_PUMP_STATE, NULL_ACTION);
   __flash item_t __flash * screen0_0items[] = {&item0scr0_0,&item1scr0_0,&item2scr0_0};
   NEW_SCR(screen0_0, "Режимы", screen0, 0);
   
   // подменю Меню\Профили
   NEW_ITEM(item0scr0_1, "Профиль 0", screen0_1_0, NULL_PARAM, NULL_ACTION);
   NEW_ITEM(item1scr0_1, "Профиль 1", screen0_1_1, NULL_PARAM, NULL_ACTION);
   NEW_ITEM(item2scr0_1, "Профиль 2", screen0_1_2, NULL_PARAM, NULL_ACTION);
   __flash item_t __flash * screen0_1items[] = {&item0scr0_1,&item1scr0_1,&item2scr0_1};
   NEW_SCR(screen0_1, "Профили", screen0, 1);
   
   //подменю Меню\Календарь
   NEW_ITEM(item0scr0_2, "Время: ", NULL_SCR,SHOW_TIME,CHANGE_TIME);
   NEW_ITEM(item1scr0_2, "Дата: ",  NULL_SCR,SHOW_DATE,CHANGE_DATE);
   __flash item_t __flash * screen0_2items[] = {&item0scr0_2,&item1scr0_2};
   NEW_SCR(screen0_2, "Календарь", screen0, 2);
      
      // подменю Меню\Режимы\Терморегуляция
      NEW_ITEM(item0scr0_0_0, "Автоматический", screen0_0_0_0, NULL_PARAM, NULL_ACTION);
      NEW_ITEM(item1scr0_0_0, "Ручной", screen0_0_0_1, NULL_PARAM, NULL_ACTION);
      __flash item_t __flash * screen0_0_0items[] = {&item0scr0_0_0,&item1scr0_0_0};
      NEW_SCR(screen0_0_0, "Терморегуляция", screen0_0, 0);

      // подменю Меню\Режимы\Вентилятор
      NEW_ITEM(item0scr0_0_1, "Автоматический", screen0_0_1_0, NULL_PARAM, NULL_ACTION);
      NEW_ITEM(item1scr0_0_1, "Ручной", screen0_0_1_1, NULL_PARAM, NULL_ACTION);
      __flash item_t __flash * screen0_0_1items[] = {&item0scr0_0_1,&item1scr0_0_1};
      NEW_SCR(screen0_0_1, "Вентилятор", screen0_0, 1);
      
      // подменю Меню\Режимы\Насос
      NEW_ITEM(item0scr0_0_2, "dT23 вкл. = ", NULL_SCR, SHOW_DT23_ON, CHANGE_DT23_ON);
      NEW_ITEM(item1scr0_0_2, "dT23 выкл. = ", NULL_SCR, SHOW_DT23_OFF, CHANGE_DT23_OFF);
      __flash item_t __flash * screen0_0_2items[] = {&item0scr0_0_2,&item1scr0_0_2};
      NEW_SCR(screen0_0_2, "Насос", screen0_0, 2);
      
      // подменю Меню\Профили\Профиль 0
      NEW_ITEM(item0scr0_1_0, "T1 = ",  NULL_SCR, SHOW_PR0_T1, CHANGE_PR0_T1);
      NEW_ITEM(item1scr0_1_0, "dT1 = ", NULL_SCR, SHOW_PR0_DT1,CHANGE_PR0_DT1);      
      NEW_ITEM(item2scr0_1_0, "T2 = ",  NULL_SCR, SHOW_PR0_T2, CHANGE_PR0_T2);
      NEW_ITEM(item3scr0_1_0, "dT2 = ", NULL_SCR, SHOW_PR0_DT2,CHANGE_PR0_DT2);
      __flash item_t __flash * screen0_1_0items[] = {&item0scr0_1_0,&item1scr0_1_0,&item2scr0_1_0,&item3scr0_1_0};
      NEW_SCR(screen0_1_0, "Профиль 0", screen0_1, 0);

      // подменю Меню\Профили\Профиль 1      
      NEW_ITEM(item0scr0_1_1, "T1 = ",  NULL_SCR, SHOW_PR1_T1, CHANGE_PR1_T1);
      NEW_ITEM(item1scr0_1_1, "dT1 = ", NULL_SCR, SHOW_PR1_DT1,CHANGE_PR1_DT1);      
      NEW_ITEM(item2scr0_1_1, "T2 = ",  NULL_SCR, SHOW_PR1_T2, CHANGE_PR1_T2);
      NEW_ITEM(item3scr0_1_1, "dT2 = ", NULL_SCR, SHOW_PR1_DT2,CHANGE_PR1_DT2);
      __flash item_t __flash * screen0_1_1items[] = {&item0scr0_1_1,&item1scr0_1_1,&item2scr0_1_1,&item3scr0_1_1};
      NEW_SCR(screen0_1_1, "Профиль 1", screen0_1, 1);
      
      // подменю Меню\Профили\Профиль 2
      NEW_ITEM(item0scr0_1_2, "T1 = ",  NULL_SCR, SHOW_PR2_T1, CHANGE_PR2_T1);
      NEW_ITEM(item1scr0_1_2, "dT1 = ", NULL_SCR, SHOW_PR2_DT1,CHANGE_PR2_DT1);      
      NEW_ITEM(item2scr0_1_2, "T2 = ",  NULL_SCR, SHOW_PR2_T2, CHANGE_PR2_T2);
      NEW_ITEM(item3scr0_1_2, "dT2 = ", NULL_SCR, SHOW_PR2_DT2,CHANGE_PR2_DT2);
      __flash item_t __flash * screen0_1_2items[] = {&item0scr0_1_2,&item1scr0_1_2,&item2scr0_1_2,&item3scr0_1_2};
      NEW_SCR(screen0_1_2, "Профиль 2", screen0_1, 2);
      
         // подменю Меню\Режимы\Терморегуляция\Автоматический
         NEW_ITEM(item0scr0_0_0_0, "Ночь: ", screen0_0_0_0_0, SHOW_NIGHT_ALL, NULL_ACTION);
         NEW_ITEM(item1scr0_0_0_0, "День: ", screen0_0_0_0_1, SHOW_DAY_ALL,   NULL_ACTION);
         __flash item_t __flash * screen0_0_0_0items[] = {&item0scr0_0_0_0,&item1scr0_0_0_0};
         NEW_SCR(screen0_0_0_0, "Автоматический", screen0_0_0, 0);
         
         // подменю Меню\Режимы\Терморегуляция\Ручной
         NEW_ITEM(item0scr0_0_0_1, "Вкл. Профиль 0", NULL_SCR, NULL_PARAM, ACTIVATE_PR_0);
         NEW_ITEM(item1scr0_0_0_1, "Вкл. Профиль 1", NULL_SCR, NULL_PARAM, ACTIVATE_PR_1);
         NEW_ITEM(item2scr0_0_0_1, "Вкл. Профиль 2", NULL_SCR, NULL_PARAM, ACTIVATE_PR_2);
         __flash item_t __flash * screen0_0_0_1items[] = {&item0scr0_0_0_1,&item1scr0_0_0_1,&item2scr0_0_0_1};
         NEW_SCR(screen0_0_0_1, "Ручной", screen0_0_0, 1);
         
         // подменю Меню\Режимы\Вентилятор\Автоматический
         NEW_ITEM(item0scr0_0_1_0, "T3 = ",  NULL_SCR, SHOW_T3, CHANGE_T3);
         NEW_ITEM(item1scr0_0_1_0, "dT3 = ", NULL_SCR, SHOW_DT3,CHANGE_DT3);
         NEW_ITEM(item2scr0_0_1_0, "Активировать", NULL_SCR, NULL_PARAM, ACTIVATE_FAN_AUTO);
         __flash item_t __flash * screen0_0_1_0items[] = {&item0scr0_0_1_0,&item1scr0_0_1_0,&item2scr0_0_1_0};
         NEW_SCR(screen0_0_1_0, "Автоматический", screen0_0_1, 0);
         
         // подменю Меню\Режимы\Вентилятор\Ручной
         NEW_ITEM(item0scr0_0_1_1, "Вкл. вентил.",  NULL_SCR, NULL_PARAM, SWITCH_FAN_ON);
         NEW_ITEM(item1scr0_0_1_1, "Выкл. вентил.", NULL_SCR, NULL_PARAM, SWITCH_FAN_OFF);
         __flash item_t __flash * screen0_0_1_1items[] = {&item0scr0_0_1_1,&item1scr0_0_1_1};
         NEW_SCR(screen0_0_1_1, "Ручной", screen0_0_1, 1);
         
            // подменю Меню\Режимы\Терморегуляция\Автоматический\Ночь
            NEW_ITEM(item0scr0_0_0_0_0, "Время вкл: ",  NULL_SCR,       SHOW_NIGHT_TIME_ON, CHANGE_NIGHT_TIME_ON);
            NEW_ITEM(item1scr0_0_0_0_0, "Профиль: ", screen0_0_0_0_0_1, SHOW_NIGHT_PROFILE, NULL_ACTION);
            __flash item_t __flash * screen0_0_0_0_0items[] = {&item0scr0_0_0_0_0,&item1scr0_0_0_0_0};
            NEW_SCR(screen0_0_0_0_0, "Ночь", screen0_0_0_0, 0);
            
            // подменю Меню\Режимы\Терморегуляция\Автоматический\День
            NEW_ITEM(item0scr0_0_0_0_1, "Время вкл: ",  NULL_SCR,       SHOW_DAY_TIME_ON, CHANGE_DAY_TIME_ON);
            NEW_ITEM(item1scr0_0_0_0_1, "Профиль: ", screen0_0_0_0_1_1, SHOW_DAY_PROFILE, NULL_ACTION);
            __flash item_t __flash * screen0_0_0_0_1items[] = {&item0scr0_0_0_0_1,&item1scr0_0_0_0_1};
            NEW_SCR(screen0_0_0_0_1, "День", screen0_0_0_0, 1);
            
               // подменю Меню\Режимы\Терморегуляция\Автоматический\Ночь\Профиль
               NEW_ITEM(item0scr0_0_0_0_0_1, "Профиль 0",  NULL_SCR, NULL_PARAM, CHANGE_NIGHT_PR_0);
               NEW_ITEM(item1scr0_0_0_0_0_1, "Профиль 1",  NULL_SCR, NULL_PARAM, CHANGE_NIGHT_PR_1);
               NEW_ITEM(item2scr0_0_0_0_0_1, "Профиль 2",  NULL_SCR, NULL_PARAM, CHANGE_NIGHT_PR_2);
               __flash item_t __flash * screen0_0_0_0_0_1items[] = {&item0scr0_0_0_0_0_1,&item1scr0_0_0_0_0_1,&item2scr0_0_0_0_0_1};
               NEW_MINOR_SCR("Ночной профиль", screen0_0_0_0_0, 1);
               //NEW_SCR(screen0_0_0_0_0_1, "Профиль", screen0_0_0_0_0, 1);
               
                // подменю Меню\Режимы\Терморегуляция\Автоматический\День\Профиль
               NEW_ITEM(item0scr0_0_0_0_1_1, "Профиль 0",  NULL_SCR, NULL_PARAM, CHANGE_DAY_PR_0);
               NEW_ITEM(item1scr0_0_0_0_1_1, "Профиль 1",  NULL_SCR, NULL_PARAM, CHANGE_DAY_PR_1);
               NEW_ITEM(item2scr0_0_0_0_1_1, "Профиль 2",  NULL_SCR, NULL_PARAM, CHANGE_DAY_PR_2);
               __flash item_t __flash * screen0_0_0_0_1_1items[] = {&item0scr0_0_0_0_1_1,&item1scr0_0_0_0_1_1,&item2scr0_0_0_0_1_1};
               NEW_SCR(screen0_0_0_0_1_1, "Дневной профиль", screen0_0_0_0_1, 1);

      

void __clr_lcd_header(void){
   uint8_t i;
   
   for(i = 0; i < 16; i++)
      *(LCD_BUFFER_P + i) = ' ';
}

void __clr_lcd_item(void){
   uint8_t i;
   for(i = 16; i < 32; i++)
      *(LCD_BUFFER_P + i) = ' ';
}

void __clr_lcd_buffer(void){
   __clr_lcd_header();
   __clr_lcd_item();
}

menu_state_t menu_get_state(void){
   return MenuState;
}

void menu_set_state(menu_state_t state){
   MenuState = state;
}

void menu_init(uint8_t * pLcdBuf){
   LCD_BUFFER_P = pLcdBuf;
   CurrItem = 0;
   pCurrScreen = INIT_SCREEN_P;
}

void __put_item_text(uint8_t * pItemBuf){
   __clr_lcd_item();
   uint8_t * pStr;
   pStr = pItemBuf;
   
   if(CurrItem != 0)
      pStr = put_char(pStr, '<');
   pStr = put_str(pStr, ITEM_TEXT(CurrItem));
   if(ITEM_PARAMETER(CurrItem) != NULL_PARAM)
      put_str_s(pStr, get_param_str(ITEM_PARAMETER(CurrItem)));
   if(CurrItem != (ITEMS_ON_SCREEN() - 1))
      put_char(pItemBuf+15, '>');
}

void __put_header_text(uint8_t * pHeaderBuf){
   uint8_t * pStr;
   pStr = put_char(pHeaderBuf, '*');
   pStr = put_str(pStr, CURR_HEADER());
   pStr = put_char(pStr, '*');
}

void menu_show_new(void){
   __clr_lcd_buffer();
   __put_header_text(LCD_HEADER_P);
   __put_item_text(LCD_ITEM_P);
}

void menu_next_item(void){
   if(CurrItem != (ITEMS_ON_SCREEN()-1)){
      CurrItem++;
      __put_item_text(LCD_ITEM_P);
   }
}

void menu_prev_item(void){   
   if(CurrItem != 0){
      CurrItem--;
      __clr_lcd_item();
      __put_item_text(LCD_ITEM_P);
   }
}

void menu_next_screen(void){
   if(NEXT_SCR_P() != NULL_SCR_P){
      pCurrScreen = NEXT_SCR_P();
      CurrItem = 0;
      menu_show_new();
   }// else {
      
}

void menu_prev_screen(void){
   if(PREV_SCR_P() != NULL_SCR_P){
      uint8_t parent_item = PARENT_ITEM();
      pCurrScreen = PREV_SCR_P();
      CurrItem = parent_item;
      menu_show_new();
   } else {
      __clr_lcd_buffer();
      MenuState = MENU_INACTIVE;
   }
}
