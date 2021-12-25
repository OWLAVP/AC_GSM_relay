/*
 */

#include "main.h"

int main(void)
{
//#if(DEBUG==0)
    _WDT_RESET(); // сброс сторожевого таймера
    WDTCSR = (1<<WDE) | (1<<WDP0) | (1<<WDP3);
//#endif
    init_uart();
    timer1_init();
    softuart_init();
    sei();
    port_init();

    init_internal_temperature();
    softuart_puts_p(PSTR("start"));
    charge_bat();
    uint8_t answer = 0;
    answer = setup_init();
    if (answer)
    {
        send_USSD("*101#");
        //softuart_puts_p(PSTR("setup_ok\r\n"));
    }
    eeprom_read_config(1);
    reset_soft_wdt();


    //delete_all_sms();


    softuart_puts_p(PSTR("end setup"));
//    uint32_t time_now = get_time_s();
    for(;;)
    {
        reset_soft_wdt();
        power_control();
        on_off_relay();
        check_registration();
        incoming_call_processing();
        check_deposit();
        get_sms();
        check_temperature();
//        if(get_time_s() > time_now + 20)
//        {
//
//            //debug_report();
////            softuart_puts_p(PSTR("delay"));
//            //char tt[5];
//            //char tt1[5];
//            // bat = analog_read(BATTERY);
////            if(bat > 699 )
////                bat = 700;
//            //sprintf(tt, "%u",get_filtered_adc(BAT_LEVEL));
//            //sprintf(tt, "%d", analog_read(BATTERY));
//            //sprintf(tt1, "%d",analog_read(5));
//            //itoa(analog_read(5), tt, 10);
//            //itoa(analog_read(8), tt1, 10);
//            //softuart_puts_ln(tt);
//            //softuart_puts_ln(tt1);
//            //map_s(bat, 615, 685, 0, 100);
////            char tt2[5];
////            itoa(map_s(bat, 610, 700, 0, 100), tt2, 10);
//            //int16_t rrr = get_filtered_adc(INT_TEMP)-318;
//            //sprintf(tt2, "%d",rrr);
////            softuart_puts_ln(tt2);
//            //DigitalLevelToggle(GREEN_LED);
//            time_now = get_time_s();
//        }
        while(is_queue_not_empty())
        {
            get_message_from_mdm();
        }
        _SLEEP();
    }


    return 0;
}
