#ifndef _SYSTEM_TIMER_H
#define _SYSTEM_TIMER_H

#define F_CPU_HZ 16000000UL      /* CPU running frequency in Hz */

#ifndef NULL
#define NULL    ((void *)0)
#endif

#define msec_timer_event_flags_t    uint8_t  /* разядность влияет на кол-во флагов событий (8 бит = 8 флагов) */
#define msec_timer_counters_t       uint16_t /* разрядность влияет на макс задержку таймера (16 бит = 65535 периодов таймера) */

#define sec_timer_event_flags_t     uint8_t  /* разядность влияет на кол-во флагов событий (8 бит = 8 флагов) */
#define sec_timer_counters_t        uint16_t /* разрядность влияет на макс задержку таймера (16 бит = 65535 периодов таймера) */

typedef void (*TimerEventCallback_t)(void);

void delay_us(uint16_t _us);
void systimer_init(void);

typedef enum{
   THERMOMETERS_POLL_EVENT = 0,
   LCD_SERVICE_EVENT,
   PUMP_ANIMATE_EVENT,
   FAN_ANIMATE_EVENT,
   HEATER_ANIMATE_EVENT,
   BUTTONS_SERVICE_EVENT
}msec_events_t;

typedef enum{
   PUMP_DELAY_EVENT = 0,
   THERMOMETERS_FAILURE_EVENT
}sec_events_t;

void msec_timer_event_config(msec_events_t ch, msec_timer_counters_t period, TimerEventCallback_t pFunction);
void msec_timer_event_en(msec_events_t ch);
void msec_timer_event_dis(msec_events_t ch);
uint8_t msec_timer_event_get(msec_events_t ch);
void msec_timer_event_clr(msec_events_t ch);

void sec_timer_event_config(sec_events_t ch, sec_timer_counters_t period, TimerEventCallback_t pFunction);
void sec_timer_event_en(sec_events_t ch);
void sec_timer_event_dis(sec_events_t ch);
uint8_t sec_timer_event_get(sec_events_t ch);
void sec_timer_event_clr(sec_events_t ch);


#endif /* _SYSTEM_TIMER_H */
