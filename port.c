
#include "main.h"

static uint8_t flag_in_volt = 0;
//*******************************************************************************************************************
void sensor_port_init(void)
{
    PinMode(PIN_CHECK_CHAPEKA, INPUT);
    DigitalWrite(PIN_CHECK_CHAPEKA, HIGH);
    PinMode(PIN_CHECK_UTKINA, INPUT);
    DigitalWrite(PIN_CHECK_UTKINA, HIGH);
}

//*******************************************************************************************************************
void unused_port_init(void)
{
    PinMode(UNUSED1, OUTPUT);
    DigitalWrite(UNUSED1, LOW);
    PinMode(UNUSED2, OUTPUT);
    DigitalWrite(UNUSED2, LOW);
    PinMode(UNUSED3, OUTPUT);
    DigitalWrite(UNUSED3, LOW);
    PinMode(UNUSED4, OUTPUT);
    DigitalWrite(UNUSED4, LOW);
    PinMode(UNUSED5, OUTPUT);
    DigitalWrite(UNUSED5, LOW);
    PinMode(UNUSED6, OUTPUT);
    DigitalWrite(UNUSED6, LOW);
//    PinMode(UNUSED7, OUTPUT);
//    DigitalWrite(UNUSED7, LOW);
    PinMode(UNUSED8, OUTPUT);
    DigitalWrite(UNUSED8, LOW);
    PinMode(UNUSED9, OUTPUT);
    DigitalWrite(UNUSED9, LOW);
}
//*******************************************************************************************************************
void relay_port_init(void)
{
    PinMode(RELAY_CHAPEKA, OUTPUT);
    DigitalWrite(RELAY_CHAPEKA, LOW);
    PinMode(RELAY_UTKINA, OUTPUT);
    DigitalWrite(RELAY_UTKINA, LOW);
}
//*******************************************************************************************************************
void led_port_init(void)
{
    PinMode(RED_LED, OUTPUT);
    DigitalWrite(RED_LED, LOW);
    PinMode(GREEN_LED, OUTPUT);
    DigitalWrite(GREEN_LED, LOW);
}

//*******************************************************************************************************************
void port_init(void)
{
    relay_port_init();
    PinMode(PWR_GSM, OUTPUT);
    DigitalWrite(PWR_GSM, LOW);
    sensor_port_init();
    led_port_init();
    unused_port_init();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//every 10 ms
//void check_volt_input(void)
//{
//    static uint8_t debounce1 = 0;
//    static uint8_t debounce2 = 0;
//    static uint8_t debounce3 = 0;
//    uint8_t temp_u = DigitalRead(PIN_CHECK_UTKINA);
//    uint8_t temp_c = DigitalRead(PIN_CHECK_CHAPEKA);
//
//    if(temp_u == 0 && temp_c == 0)
//    {
//        if(debounce1 < 10) // 30ms
//            debounce1++;
//        else
//        {
//            flag_in_volt = IS_VOLT_ALL;
//            debounce1 = 0;
//        }
//    }
//    else if(temp_c == 0)
//    {
//        if(debounce2 < 10)
//            debounce2++;
//        else
//        {
//            flag_in_volt = IS_VOLT_CHAPEKA;
//            debounce2 = 0;
//        }
//    }
//    else if( temp_u == 0)
//    {
//        if(debounce3 < 10)
//            debounce3++;
//        else
//        {
//            flag_in_volt = IS_VOLT_UTKINA;
//            debounce3 = 0;
//        }
//    }
//    else
//    {
//            flag_in_volt = NOT_VOLT;
//            debounce1 = 0;
//            debounce2 = 0;
//            debounce3 = 0;
//    }
//}

void check_volt_input(void)
{
    uint8_t temp_u = DigitalRead(PIN_CHECK_UTKINA);
    uint8_t temp_c = DigitalRead(PIN_CHECK_CHAPEKA);

    if(temp_u == 0 && temp_c == 0)
    {
            flag_in_volt = IS_VOLT_ALL;
    }
    else if(temp_c == 0)
    {
            flag_in_volt = IS_VOLT_CHAPEKA;
    }
    else if( temp_u == 0)
    {
            flag_in_volt = IS_VOLT_UTKINA;
    }
    else
    {
            flag_in_volt = NOT_VOLT;
    }
}
//*******************************************************************************************
uint8_t get_flag_input_voltage(void)
{
    return get_val(flag_in_volt);
}






