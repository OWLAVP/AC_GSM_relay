

#ifndef TIMER_H_
#define TIMER_H_

void timer1_init(void);
uint32_t get_time_ms(void);
uint32_t get_time_s(void);
uint32_t get_time_m(void);
void delay_ms(uint16_t delay);
void delay_s(uint8_t delay);
void reset_soft_wdt(void);
uint32_t get_time_from_start(void);

void set_time_from_change_s(uint8_t time);
uint32_t get_time_from_change_s();

uint32_t get_time_from_start_s();
void set_time_from_start_s(uint32_t time);


#endif /* TIMER_H_ */
