#include "main.h"

static uint32_t time_without_power_s = 0;


static char is_external_pwr(void);
static uint8_t level_battery(void);
//****************************************************************************************************


//****************************************************************************************************
void on_off_relay()
{
    static uint8_t last_state_input = IS_VOLT_ALL;
    static uint8_t temp_state_input = 0;
    static uint8_t last_gsm_cmd = 0;
    static uint32_t time_stamp_s = 0;
    uint8_t state = get_flag_input_voltage();
    uint8_t gsm_cmd = get_mode_gsm();

    if(state != temp_state_input)
    {
        time_stamp_s = get_time_s() + get_pause_relay();// sensitivity
        temp_state_input = state;
        //softuart_puts_p(PSTR("change"));
        return;
    }
    else if((get_time_s() < time_stamp_s))
    {
        return;
    }
    else if(last_gsm_cmd == gsm_cmd && state == last_state_input)
        return;

    if(gsm_cmd == GSM_ON_CHAPEKA)
    {
        last_gsm_cmd = gsm_cmd;
        if(state == IS_VOLT_ALL)
        {
//            DigitalWrite(RELAY_UTKINA, HIGH);
//            delay_ms(300);
            DigitalWrite(RELAY_CHAPEKA, HIGH);
 //           delay_ms(300);
            last_state_input = IS_VOLT_ALL;
            softuart_puts_p(PSTR("ENERGY +++\r\n"));
            if(get_accept_sms() == 1)
                send_sms_admins(PSTR("ENERGY +++"));
        }
        else if(state == IS_VOLT_UTKINA)
        {
            DigitalWrite(RELAY_CHAPEKA, LOW);
//            delay_ms(300);
//            DigitalWrite(RELAY_UTKINA, LOW);
//            delay_ms(300);
            last_state_input = IS_VOLT_UTKINA;
            softuart_puts_p(PSTR("ENERGY FROM UTKINA\r\n"));
            if(get_accept_sms() == 1)
                send_sms_admins(PSTR("ENERGY FROM UTKINA"));
        }
        else if(state == IS_VOLT_CHAPEKA)
        {
//            DigitalWrite(RELAY_UTKINA, HIGH);
//            delay_ms(300);
            DigitalWrite(RELAY_CHAPEKA, HIGH);
//            delay_ms(300);
            last_state_input = IS_VOLT_CHAPEKA;
            softuart_puts_p(PSTR("ENERGY FROM CHAPEKA\r\n"));
            if(get_accept_sms() == 1)
                send_sms_admins(PSTR("ENERGY FROM CHAPEKA"));
        }
        else if(state == NOT_VOLT)
        {
            softuart_puts_p(PSTR("NOT_ENERGY\r\n"));
            set_mode_gsm(GSM_ON_UTKINA);
            EEPROM_update();
            DigitalWrite(RELAY_CHAPEKA, LOW);
//            delay_ms(300);
//            DigitalWrite(RELAY_UTKINA, LOW);
//            delay_ms(300);
            if(get_accept_calls() == 1)
                alarm_play_file(ALL_NOT);
            last_state_input = NOT_VOLT;
            return;
        }
    }
    else if (gsm_cmd == GSM_ON_UTKINA)
    {
        last_gsm_cmd = gsm_cmd;
        if (state == IS_VOLT_ALL)
        {
            DigitalWrite(RELAY_CHAPEKA, LOW);
//            delay_ms(300);
//            DigitalWrite(RELAY_UTKINA, LOW);
//            delay_ms(300);
            last_state_input = IS_VOLT_ALL;
            softuart_puts_p(PSTR("ENERGY +++\r\n"));
            if(get_accept_sms() == 1)
                send_sms_admins(PSTR("ENERGY +++"));
        }
        else if (state == IS_VOLT_CHAPEKA)
        {
//            DigitalWrite(RELAY_UTKINA, HIGH);
//            delay_ms(300);
            DigitalWrite(RELAY_CHAPEKA, HIGH);
//            delay_ms(300);
            last_state_input = IS_VOLT_CHAPEKA;
            softuart_puts_p(PSTR("ENERGY FROM CHAPEKA\r\n"));
            if(get_accept_sms() == 1)
                send_sms_admins(PSTR("ENERGY FROM CHAPEKA"));
        }
        else if (state == IS_VOLT_UTKINA)
        {
            DigitalWrite(RELAY_CHAPEKA, LOW);
//            delay_ms(300);
//            DigitalWrite(RELAY_UTKINA, LOW);
//            delay_ms(300);
            last_state_input = IS_VOLT_UTKINA;
            softuart_puts_p(PSTR("ENERGY FROM UTKINA\r\n"));
            if(get_accept_sms() == 1)
                send_sms_admins(PSTR("ENERGY FROM UTKINA"));
        }
        else if(state == NOT_VOLT)
        {
            softuart_puts_p(PSTR("NOT_ENERGY\r\n"));
            DigitalWrite(RELAY_CHAPEKA, LOW);
//            delay_ms(300);
//            DigitalWrite(RELAY_UTKINA, LOW);
//            delay_ms(300);
            if(get_accept_calls() == 1)
                alarm_play_file(ALL_NOT);
            last_state_input = NOT_VOLT;
            return;
        }
    }
//    else if (gsm_cmd == GSM_REBOOT_LAN)
//    {
//        set_mode_gsm(GSM_ON_UTKINA);
//        DigitalWrite(RELAY_UTKINA, HIGH);
//        delay_ms(300);
//        DigitalWrite(RELAY_CHAPEKA, LOW);
//        delay_s(20);
//        softuart_puts_p(PSTR("GSM_REBOOT_LAN\r\n"));
//    }

}
//******************************************************************************************************************
static char is_external_pwr(void)
{
    uint8_t temp = get_flag_input_voltage();
    if(temp != NOT_VOLT)
        return 1;
    else
        return 0;
}

//*******************************************************************************************************************

void check_power(void)
{
    if(is_external_pwr())
        time_without_power_s = 0;
    else
        time_without_power_s++;
}
//*******************************************************************************************************************
void power_control(void)
{
    static uint32_t time_stamp_on = 0;
    static uint32_t time_stamp_off = 0;
    static uint32_t time_stamp_bat = 0;
    static uint8_t report_on = 0;
    static uint8_t report_off = 0;
    static uint8_t report_bat = 0;
    static uint8_t flag_time_on = 1;
    static uint8_t flag_time_off = 1;
    static uint8_t flag = 0;

    if(time_without_power_s == 0)   // power+++
    {
        if(get_interval_power_on_report() != 0)
        {
            if(flag == 0)
            {
                flag = 1;
                softuart_puts_p(PSTR("power +++."));
            }
            if(flag_time_on == 0)
            {
                time_stamp_on = get_time_s();
                flag_time_on = 1;
                report_on = 1;
            }
            if(report_on == 1)
            {
                if(flag_time_on == 1 && (time_stamp_on + (uint32_t)60*get_interval_power_on_report()) < get_time_s())
                {
                    report_on = 0;
                    flag_time_off = 1;
                    softuart_puts_p(PSTR("External power is on."));
                    if(get_accept_sms() == 1)
                        send_sms_admins(PSTR("External power is on."));
                }
            }
        }
        return;
    }
    if(get_interval_power_off_report() != 0)  ///  power---
    {
        time_stamp_off = get_val(time_without_power_s);
        time_stamp_on = get_time_s();
        if(flag == 1)
        {
            flag = 0;
            softuart_puts_p(PSTR("power ---"));
        }
        if(flag_time_off == 1 && time_stamp_off > (uint32_t)60*get_interval_power_off_report())
        {
            report_off = 1;
        }
        if(report_off == 1)
        {
            report_off = 0;
            flag_time_on = 0;
            flag_time_off = 0;
            report_bat = 1;
            time_stamp_bat = get_time_s();
            softuart_puts_p(PSTR("External power is off."));
            if(get_accept_sms() == 1)
                send_sms_admins(PSTR("External power is off."));
        }
        if(report_bat == 1 && (get_time_s() > (time_stamp_bat + 120)))
        {
            softuart_puts_p(PSTR("check battery"));
            time_stamp_bat = get_time_s();
//            char tt[5];
//            sprintf(tt, "bat=%d%%",level_battery());
//            softuart_puts_ln(tt);
            if(level_battery() < 25)
            {
                report_bat = 0;
                if(get_accept_sms() == 1)
                    send_sms_admins(PSTR("Battery LOW!!!"));
                softuart_puts_p(PSTR("low battery"));
            }
        }
    }
}
///****************************************************************************
static uint8_t level_battery(void)
{
    int16_t bat = analog_read(BATTERY);
    if(bat < 609)
    {
        set_mode_gsm(GSM_ON_UTKINA);
        EEPROM_update();
        reset_mcu();
        return 0;
    }
    else if(bat > 699)
        bat = 700;
    return map_s(bat, 610, 700, 0, 100);
}
//*****************************************************************************
void charge_bat(void)
{
    while(analog_read(BATTERY) < 609)
    {
        reset_soft_wdt();
        delay_s(5);
    }
}
//*****************************************************************************
uint32_t get_time_without_power_s(void)
{
    return time_without_power_s;
}
//*****************************************************************************
void send_sms_report(char *phone)
{
    uint32_t time;
    uint16_t d, h, m, s;
    char sms[160];
    char *ptr;

    time = get_time_from_start_s();
    d = time/(3600UL*24UL);
    time -= d*(3600UL*24UL);
    h = time/3600UL;
    time -= h*3600UL;
    m = time/60;
    s = time - m*60;
    ptr = sms;
    uint8_t in_volt = get_flag_input_voltage();

    send_USSD("*101#");

    ptr += sprintf_P(ptr, PSTR("Deposit=%s RUB, "), get_arr_dep());
    ptr += sprintf_P(ptr, PSTR("Time=%ud%uh%um%us, "), d, h, m, s);
    ptr += sprintf_P(ptr, PSTR("Temperature=%dC, "), analog_read(INT_TEMP));
    ptr += sprintf_P(ptr, PSTR("GSM=%d%%, "), mdm_get_signal_strength());

    if(get_time_without_power_s() != 0)
    {
        ptr += sprintf_P(ptr, PSTR("Battery=%d%%, "), level_battery());
    }

    ptr += sprintf_P(ptr, PSTR("rst_mcu=%lu. "), get_reset_count_mcu());

    if(in_volt == IS_VOLT_ALL)
    {
        ptr = strcat(ptr, "All good, ");
        if(get_mode_gsm() == GSM_ON_CHAPEKA)
        {
            ptr = strcat_P(ptr, PSTR("energy from Chapeka."));
        }
        else if(get_mode_gsm() == GSM_ON_UTKINA)
        {
            ptr = strcat_P(ptr, PSTR("energy from Utkina."));
        }
    }
    else if(in_volt == IS_VOLT_CHAPEKA)
    {
        ptr = strcat_P(ptr, PSTR("Warning! Energy from Chapeka."));
    }
    else if(in_volt == IS_VOLT_UTKINA)
    {
        ptr = strcat_P(ptr, PSTR("Warning! Energy from Utkina."));
    }
    else if(in_volt == NOT_VOLT)
    {
        ptr = strcat_P(ptr, PSTR("Alarm! Not energy!!!"));
    }
    send_sms(sms, phone);
}
//*****************************************************************************
void debug_report(void)
{
    uint32_t time;
    uint16_t d, h, m, s;
    char sms[100];
    char *ptr;

    time = get_time_from_start_s();
    d = time/(3600UL*24UL);
    time -= d*(3600UL*24UL);
    h = time/3600UL;
    time -= h*3600UL;
    m = time/60;
    s = time - m*60;
    ptr = sms;
    uint8_t in_volt = get_flag_input_voltage();

    ptr += sprintf_P(ptr, PSTR("Dep=%s, "), get_arr_dep());

    ptr += sprintf_P(ptr, PSTR("time=%ud%uh%um%us, "), d, h, m, s);

    ptr += sprintf_P(ptr, PSTR("temp=%dC, "), analog_read(INT_TEMP));

    ptr += sprintf_P(ptr, PSTR("rst_mcu=%lu, "), get_reset_count_mcu());

    ptr += sprintf_P(ptr, PSTR("s=%d%%, "), mdm_get_signal_strength());

    uint8_t time_w_p = (uint8_t)get_time_without_power_s();
    if(time_w_p == 0)
    {
        ptr += sprintf_P(ptr, PSTR("e_power=%u. "), 1);
    }
    else
    {
        ptr += sprintf_P(ptr, PSTR("battery=%d%%. "), level_battery());

    }
    if(in_volt == IS_VOLT_ALL)
    {
        ptr = strcat(ptr, "All Ok, ");
        if(get_mode_gsm() == GSM_ON_CHAPEKA)
        {
            ptr = strcat_P(ptr, PSTR("energy from Chapeka."));
        }
        else if(get_mode_gsm() == GSM_ON_UTKINA)
        {
            ptr = strcat_P(ptr, PSTR("energy from Utkina."));
        }
    }
    else if(in_volt == IS_VOLT_CHAPEKA)
    {
        ptr = strcat_P(ptr, PSTR(" Warning! Energy from Chapeka."));
    }
    else if(in_volt == IS_VOLT_UTKINA)
    {
        ptr = strcat_P(ptr, PSTR(" Warning! Energy from Utkina."));
    }
    else if(in_volt == NOT_VOLT)
    {
        ptr = strcat_P(ptr, PSTR(" Alarm! Not energy!!!"));
    }
    softuart_puts_ln(sms);
    print_freeRam();
}

//********************************************************************************************************
void init_internal_temperature(void)
{
    ADMUX |= (1<<REFS1) | (1<<REFS0); //Internal 1.1V Voltage Reference
    if(F_CPU == 16000000)
    {
        ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // 16Mhz F_CPU / 128 prescaler = 125Khz ADC clock source
    }
    else if(F_CPU == 7372800)
    {
        ADCSRA |= (1 << ADPS2) | (1 << ADPS1); // 8Mhz F_CPU / 64 prescaler = 125Khz ADC clock source
        ADCSRA &= ~(1 << ADPS0);
    }
    else
    {
        ADCSRA |= (1 << ADPS1) | (1 << ADPS0); // 1Mhz F_CPU / 8 prescaler = 125Khz ADC clock source
        ADCSRA &= ~(1 << ADPS2);
    }
}


//*******************************************************************************************************************


//********************************************************************************************************************
void check_temperature(void)
{

    static uint32_t time_stamp_s = 0;
    static uint8_t flag = 0;
    if(flag == 1)
        return;
    if(get_report_temperature() == 0)
        return;
    if(get_time_s() > (time_stamp_s + 60))
    {
        time_stamp_s = get_time_s();
//        char tt[5];
//        itoa(analog_read(INT_TEMP), tt, 10);
//        softuart_puts_ln(tt);
        if(analog_read(INT_TEMP) > 70)
        {
            flag = 1;
            softuart_puts_p(PSTR("Alarm! TEMPERATURE!!!"));
//            if(get_accept_calls() == 1)
//                alarm_play_file(ALARM_TEMPERATURE);
            if(get_accept_sms() == 1)
                send_sms_admins(PSTR("Alarm! TEMPERATURE > 70"));
        }
    }
}

//***********************************************************************************************
int16_t analog_read(uint8_t input)
{

    uint16_t ADC_cache=0, i;
    long buffer=0;
    ADMUX &=  (~(1<<MUX3)) & (~(1<<MUX2)) & (~(1<<MUX1)) & (~(1<<MUX0));
    // Select the ADC reference voltage and channel. Must be done everytime because ADMUX can be overwritten in meantime
    if(input == INT_TEMP)
        ADMUX |= (1<<MUX3);         // ADC channel 8 - temperature sensor
    else if(input == BATTERY)
        ADMUX |= (1<<MUX2) | (1<<MUX0);
    ADCSRA |= (1 << ADEN);  // Power up the ADC
    ADCSRA |= (1 << ADIE);  // Enable the ADC Interrupt

    SMCR |= (1 << SM0);    // ADC Noise Reduction Sleep Mode
    delay_ms(20);         // Wait for voltages to become stable
    ADCSRA |= (1 << ADSC);  // Start converting

    // Take a reading and discard the first sample to prevent errors
    while(ADCSRA & (1 << ADSC));

    for(i=CHIP_TEMP_NR_OF_SAMPLES; i>0; i--)
    {
        // Enter Sleep Mode (ADC Noise Reduction Sleep Mode) and Start a new conversion
        SMCR |= (1 << SE);

        // Wait for ADC conversion
        while(ADCSRA & (1 << ADSC)); // ADSC is cleared when the conversion finishes

        // After the Sleep Mode this bit must be set to 0 according to the datasheet
        SMCR &= ~(1 << SE);

        // Read register atomically in case other interrupts are enabled
        cli();
        ADC_cache = ADCW;
        sei();

        buffer += ADC_cache;
    }
    //print_freeRam();
    // Shut down the ADC to save power
    ADCSRA &= ~(1 << ADEN);
    if(input == INT_TEMP)
        return ((buffer / CHIP_TEMP_NR_OF_SAMPLES) + CHIP_TEMP_ERROR_OFFSET) - 273;
    else if(input == BATTERY)
        return (buffer / CHIP_TEMP_NR_OF_SAMPLES);
    return 0;
}

ISR(ADC_vect)
{

}

//*****************************************************************************
static int freeRam ();
static int freeRam ()
{
    extern int __heap_start, *__brkval;
    int v;
    return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}


void print_freeRam(void)
{
    char ch[10];
    itoa(freeRam(), ch, 10);
    softuart_puts("free=");
    softuart_puts_ln(ch);
}
