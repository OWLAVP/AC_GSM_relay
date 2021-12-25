
#include "main.h"

static uint32_t time_ms=0;
static uint32_t time_s=0;
static uint32_t time_m=0;
static uint16_t soft_wdt = 0;
static uint32_t time_from_start_s=0; // таймер с начала работы прибора

//*******************************************************************************************************************

// период переполнения 10 мс
void timer1_init(void)
{
    cli(); // stop interrupts
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;

    // 100 Hz (7372800/((71+1)*1024))
    OCR1A = 71;
    // CTC
    TCCR1B |= (1 << WGM12);
    // Prescaler 1024
    TCCR1B |= (1 << CS12) | (1 << CS10);
    // Output Compare Match A Interrupt Enable
    TIMSK1 |= (1 << OCIE1A);
}

//*******************************************************************************************************************

// прерывание вызывается с периодом 10 мс
ISR(TIMER1_COMPA_vect)
{
    static uint8_t i=0;
    static uint8_t j=0;

    time_ms+=10;
    i++;
    check_volt_input();
    if(i>=100) // 1 секунда
    {
        i=0;
        time_s++;
        time_from_start_s++;
        soft_wdt++;
        j++;
        DigitalLevelToggle(RED_LED);
        check_power();
        if(j>=60)
        {
            j=0;
            time_m++;
        }
        if(soft_wdt > 120) //  sec
            reset_mcu();
    }
    _WDT_RESET(); // сброс сторожевого таймера
}

//*******************************************************************************************************************

uint32_t get_time_ms(void)
{
    return get_val(time_ms);
}
//*******************************************************************************************************************

uint32_t get_time_s(void)
{
    return get_val(time_s);
}

//*******************************************************************************************************************

uint32_t get_time_m(void)
{
    return get_val(time_m);
}

//*******************************************************************************************************************

void reset_soft_wdt(void)
{
    set_val(soft_wdt, 0);
}

//*******************************************************************************************************************
// задержка на х милисекунд
void delay_ms(uint16_t delay)
{
    uint32_t time_stamp = get_time_ms() + delay;
    while((get_time_ms() < time_stamp))
    {
        _SLEEP();
        while(is_queue_not_empty()) // тем временем, проверяем входной буфер
            get_message_from_mdm();
    }
}

//*******************************************************************************************************************
// задержка на х секунд
void delay_s(uint8_t delay)
{
    uint32_t time_stamp = get_time_s() + delay;
    while((get_time_s() < time_stamp))
    {
        _SLEEP();
        while(is_queue_not_empty()) // тем временем, проверяем входной буфер
            get_message_from_mdm();
    }
}


//*******************************************************************************************************************


/////////////////////// time_from_start_s //////////////////////////
uint32_t get_time_from_start_s()
{
    return get_val(time_from_start_s);
}





