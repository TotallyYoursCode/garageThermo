#include <ioavr.h>
#include <intrinsics.h>
#include <stdint.h>
#include "systimer.h"
#include "bit.h"

#include "debug.h"

#define SYSTIMER_PERIOD_ms       1
#define SYSTIMER_BIT             8
#define SYSTIMER_PRESCALER       64  /* 0, 1, 8, 64, 256, 1024 */
#define SYSTIMER_MAX             (1<<SYSTIMER_BIT)
#define SYSTIMER_TICKS           (((SYSTIMER_PERIOD_ms*F_CPU_HZ)/1000)/SYSTIMER_PRESCALER)
#if(SYSTIMER_TICKS >= SYSTIMER_MAX)
#error "Too large SYSTIMER_PERIOD_ms or too small SYSTIMER_PRESCALER for F_CPU_HZ value"
#elif(SYSTIMER_TICKS <= 0)
#error "Too large SYSTIMER_PRESCALER or too small SYSTIMER_PERIOD_ms for F_CPU_HZ value"
#endif
#define SYSTIMER_RELOAD          (SYSTIMER_MAX - SYSTIMER_TICKS)
#if (SYSTIMER_PRESCALER==0)
#define SYSTIMER_CS00 0
#elif (SYSTIMER_PRESCALER==1)
#define SYSTIMER_CS00 1
#elif (SYSTIMER_PRESCALER==8)
#define SYSTIMER_CS00 2
#elif (SYSTIMER_PRESCALER==64)
#define SYSTIMER_CS00 3
#elif (SYSTIMER_PRESCALER==256)
#define SYSTIMER_CS00 4
#elif (SYSTIMER_PRESCALER==1024)
#define SYSTIMER_CS00 5
#else
#error "SYSTIMER_PRESCALER - недопустимое значение"
#endif

#define SYSTIMER_ONE_TICK_PERIOD_us    (SYSTIMER_PERIOD_ms*1000/SYSTIMER_TICKS)


#define  MSEC_TIMER_MAX_EVENTS      (sizeof(msec_timer_event_flags_t)*8)

static   msec_timer_event_flags_t   MsecEventEnFlags, MsecEventFlags;
static   msec_timer_counters_t      MsecEventCounters[MSEC_TIMER_MAX_EVENTS],MsecEventCountersMax[MSEC_TIMER_MAX_EVENTS];
volatile TimerEventCallback_t       MsecEventCallbackFunction[MSEC_TIMER_MAX_EVENTS];


#define  SEC_TIMER_MAX_EVENTS       (sizeof( sec_timer_event_flags_t)*8)

static   sec_timer_event_flags_t    SecEventEnFlags, SecEventFlags;
static   sec_timer_counters_t       SecEventCounters[SEC_TIMER_MAX_EVENTS],SecEventCountersMax[SEC_TIMER_MAX_EVENTS];
volatile TimerEventCallback_t       SecEventCallbackFunction[SEC_TIMER_MAX_EVENTS];

typedef struct{
   uint8_t  Initialized:1,
            bit1:1,
            bit2:1,
            bit3:1,
            bit4:1,
            bit5:1,
            bit6:1,
            bit7:1;
}systimer_flag_t;
static systimer_flag_t Systimer;
static volatile uint16_t MsecCnt = 0;

void delay_ms(uint16_t _ms){
   if(!Systimer.Initialized){
      systimer_init();
      Systimer.Initialized = 1;
   }
   uint16_t PrevMsecCnt = MsecCnt;
   while(((MsecCnt < PrevMsecCnt) ? (1000 + MsecCnt - PrevMsecCnt) : (MsecCnt - PrevMsecCnt)) < _ms);
}


//void delay_us(uint16_t _us){
//   while(_us--) __delay_cycles(F_CPU_HZ/1250000);//1458454);  /* особая уличная магия :) работает только в IAR при максимальной оптимизации... на разных IAR по разному :( */
//}

void delay_us(uint8_t us){   
   if(!Systimer.Initialized){
      systimer_init();
      Systimer.Initialized = 1;
   }
   uint8_t PrevTickCnt = TCNT0;
   while(((TCNT0 < PrevTickCnt) ? (255 + TCNT0 - PrevTickCnt) : (TCNT0 - PrevTickCnt)) < (us/SYSTIMER_ONE_TICK_PERIOD_us));
}

void msec_timer_init(void){
   uint8_t i;
   MsecEventEnFlags = 0;
   MsecEventFlags = 0;
   for(i = 0; i < MSEC_TIMER_MAX_EVENTS; i++){
      MsecEventCounters[i] = 0;
      MsecEventCountersMax[i] = ~0;
      MsecEventCallbackFunction[i] = NULL;
   }
}

void sec_timer_init(void){
#ifdef __DEBUG_HEADER_FILE
   debug_init(DEBUG_WIRE);
#endif
   uint8_t i;
   SecEventEnFlags = 0;
   SecEventFlags = 0;
   for(i = 0; i < SEC_TIMER_MAX_EVENTS; i++){
      SecEventCounters[i] = 0;
      SecEventCountersMax[i] = ~0;
      SecEventCallbackFunction[i] = NULL;
   }
}

void systimer_init(void){
   TIMSK &= ~((1<<TOIE0)|(1<<OCIE0));        /* запрет прерывания таймера */
   TCCR0 &= ~(7<<CS00);                      /* остановка таймера */
   TIFR  |=  ((1<<OCF0)|(1<<TOV0));          /* сброс флагов прерываний */
   TCNT0  =  SYSTIMER_RELOAD;                /* настройка периода таймера */
   TIMSK |=  (1<<TOIE0);                     /* разрешение прерывания таймера */
   TCCR0 |=  (SYSTIMER_CS00<<CS00);          /* запуск таймера */   
   msec_timer_init();                        /* инициализация программного таймера милисекунд */
   sec_timer_init();                         /* инициализация программного таймера секунд */
}

void msec_timer_event_config(msec_events_t ch, msec_timer_counters_t period, TimerEventCallback_t pFunction){
   if(ch >= MSEC_TIMER_MAX_EVENTS)
      ch = (msec_events_t)(MSEC_TIMER_MAX_EVENTS - 1);
   bit_clr(MsecEventEnFlags, ch);
   MsecEventCallbackFunction[ch] = pFunction;
   MsecEventCountersMax[ch] = period;
   MsecEventCounters[ch] = 0L;
   bit_clr(MsecEventFlags, ch);
   bit_set(MsecEventEnFlags, ch);
}

void msec_timer_event_en(msec_events_t ch){
   if(ch >= MSEC_TIMER_MAX_EVENTS)
      ch = (msec_events_t)(MSEC_TIMER_MAX_EVENTS - 1);
   MsecEventCounters[ch] = 0L; 
   bit_set(MsecEventEnFlags, ch);
}

void msec_timer_event_dis(msec_events_t ch){
   if(ch >= MSEC_TIMER_MAX_EVENTS)
      ch = (msec_events_t)(MSEC_TIMER_MAX_EVENTS - 1);
   bit_clr(MsecEventEnFlags, ch);
}

uint8_t msec_timer_event_get(msec_events_t ch){
   if(ch >= MSEC_TIMER_MAX_EVENTS)
      ch = (msec_events_t)(MSEC_TIMER_MAX_EVENTS - 1);
   return (bit_test(MsecEventFlags, ch));
}

void msec_timer_event_clr(msec_events_t ch){
   if(ch >= MSEC_TIMER_MAX_EVENTS)
      ch = (msec_events_t)(MSEC_TIMER_MAX_EVENTS - 1);
   MsecEventCounters[ch] = 0L;
   bit_clr(MsecEventFlags, ch);
}

void sec_timer_event_config(sec_events_t ch, sec_timer_counters_t period, TimerEventCallback_t pFunction){
   if(ch >= SEC_TIMER_MAX_EVENTS)
      ch = (sec_events_t)(SEC_TIMER_MAX_EVENTS - 1);
   bit_clr(SecEventEnFlags, ch);
   SecEventCallbackFunction[ch] = pFunction;
   SecEventCountersMax[ch] = period;
   SecEventCounters[ch] = 0L;
   bit_clr(SecEventFlags, ch);
   bit_set(SecEventEnFlags, ch);
}

void sec_timer_event_en(sec_events_t ch){
   if(ch >= SEC_TIMER_MAX_EVENTS)
      ch = (sec_events_t)(SEC_TIMER_MAX_EVENTS - 1);
   SecEventCounters[ch] = 0L; 
   bit_set(SecEventEnFlags, ch);
}

void sec_timer_event_dis(sec_events_t ch){
   if(ch >= SEC_TIMER_MAX_EVENTS)
      ch = (sec_events_t)(SEC_TIMER_MAX_EVENTS - 1);
   bit_clr(SecEventEnFlags, ch);
}

uint8_t sec_timer_event_get(sec_events_t ch){
   if(ch >= SEC_TIMER_MAX_EVENTS)
      ch = (sec_events_t)(SEC_TIMER_MAX_EVENTS - 1);
   return (bit_test(SecEventFlags, ch));
}

void sec_timer_event_clr(sec_events_t ch){
   if(ch >= SEC_TIMER_MAX_EVENTS)
      ch = (sec_events_t)(SEC_TIMER_MAX_EVENTS - 1);
   SecEventCounters[ch] = 0L;
   bit_clr(SecEventFlags, ch);
}

#pragma vector=TIMER0_OVF_vect
__interrupt void SYSTIMER_OVF(){
   TCNT0  = SYSTIMER_RELOAD;                 /* настройка периода таймера */
#ifdef __DEBUG_HEADER_FILE
   debug_on(DEBUG_WIRE);
#endif
   uint8_t i;
   for(i = 0; i < MSEC_TIMER_MAX_EVENTS; i++){
      if(bit_test(MsecEventEnFlags, i)){
         MsecEventCounters[i]++;            /* Increment millisecond event counter.*/
         if(MsecEventCounters[i] == MsecEventCountersMax[i]){
            MsecEventCounters[i] = 0L; 
            bit_set(MsecEventFlags, i);
            if(MsecEventCallbackFunction[i] != NULL){
               MsecEventCallbackFunction[i]();
            }
         }
      }
   }
   if(++MsecCnt >= 1000){
      MsecCnt = 0;
      for(i = 0; i < SEC_TIMER_MAX_EVENTS; i++){
         if(bit_test(SecEventEnFlags, i)){
            SecEventCounters[i]++;            /* Increment second event counter.*/
            if(SecEventCounters[i] == SecEventCountersMax[i]){
               SecEventCounters[i] = 0L; 
               bit_set(SecEventFlags, i);
               if(SecEventCallbackFunction[i] != NULL){
                  SecEventCallbackFunction[i]();
               }
            }
         }
      }
   }
#ifdef __DEBUG_HEADER_FILE
   debug_off(DEBUG_WIRE);
#endif
}


#pragma vector=TIMER0_COMP_vect
__interrupt void T0_COMP()
{   
}
