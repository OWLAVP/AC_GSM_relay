

#include "main.h"


#define SIZE_OF_SMS_TEXT_BUF 320
#define MAX_SMS_LENGTH       160


static const char help_text[MAX_SMS_LENGTH+1] PROGMEM = {"helpset;helpget;report;adminphones;reboot;resetdefault;"};
static const char help_get[MAX_SMS_LENGTH+1] PROGMEM = {"get:adminphones;accept_calls;accept_sms;report_t;"};
static const char help_set[MAX_SMS_LENGTH+1] PROGMEM = {"set:accept_calls=;accept_sms=;report_t=;utkina=;chapeka=;pause_relay=;"};

static char* set_phones(char *phones, char *dest, char max_num);

char find_phone_in_phone_list(char *phone, char lst)
{
    uint8_t i;

    if(lst==ADMIN_LIST)
    {
        for(i=0; i<TOTAL_ADMIN_NUMBER; i++)
        {
            if(config.admin_phone[i][0] == '+')
                if(memcmp(phone, &config.admin_phone[i][0], 13) == 0)
                {
                    softuart_puts_p(PSTR("phone ok"));
                    return 1; // телефон найден в списке админов
                }
        }
    }
    softuart_puts_p(PSTR("not phone"));
    return 0;
}

//*******************************************************************************************************************


void process_sms_body(char *ptr)
{
    uint8_t i, err;

    //beep_ms(10);
    ptr = strlwr(ptr);
    if(memcmp_P(ptr, PSTR("set:"), 4) == 0)
    {
        if((find_phone_in_phone_list(get_phone_number(), ADMIN_LIST) == 0))
        {
            send_sms_p(PSTR("Access denied"), get_phone_number());
            return;
        }
        ptr += 4;
        i=0;
        err = 0;
        while(ptr && *ptr)
        {
            ptr = set_param(ptr);
            //softuart_puts_ln(ptr);
            if(ptr)
                i++;
            else
                err = 1;
        }
        if(i && !err)
        {
            EEPROM_update();
            send_sms_p(PSTR("ok"), get_phone_number());
        }
        else
        {
            eeprom_read_config(0); // возвращаем всё в зад
            send_sms_p(PSTR("error"), get_phone_number());
        }
    }


    else if(memcmp_P(ptr, PSTR("get:"), 4) == 0)
    {
        char sms_text[SIZE_OF_SMS_TEXT_BUF];
        if((find_phone_in_phone_list(get_phone_number(), ADMIN_LIST) == 0))
        {
            send_sms_p(PSTR("Access denied"), get_phone_number());
            return;
        }
        ptr += 4;
        sms_text[0] = 0; // терминируем строку
        i=0;
        err = 0;

        while(ptr && *ptr)
        {

            ptr = get_param(ptr, &sms_text[strlen(sms_text)]);
            // softuart_puts_ln(ptr);
            if(ptr)
                i++;
            else
            {
                err = 1;
                break;
            }
            if(strlen(sms_text) > MAX_SMS_LENGTH)
            {
                err = 2;
                break;
            }
        }
        if(i && !err)
        {
            send_sms(sms_text, get_phone_number());
            softuart_puts_p(PSTR("send_sms\r\n"));
        }

        else if(err==1)
        {
            send_sms_p(PSTR("error"), get_phone_number());
        }

        else if(err==2)
        {
            send_sms_p(PSTR("resulting SMS text is too long"), get_phone_number());
        }

    }

    else if(memcmp_P(ptr, PSTR("resetdefault;"), 13) == 0)
    {
        if((find_phone_in_phone_list(get_phone_number(), ADMIN_LIST) == 0))
        {
            send_sms_p(PSTR("Access denied"), get_phone_number());
            return;
        }
        set_first_usage(0);
        EEPROM_update();
        send_sms_p(PSTR("Device reset default, send adminphones=NUMBER_PHONES"), get_phone_number());
        reset_mcu();
    }

    else if(memcmp_P(ptr, PSTR("reboot;"), 7) == 0)
    {
        if((find_phone_in_phone_list(get_phone_number(), ADMIN_LIST) == 0))
        {
            send_sms_p(PSTR("Access denied"), get_phone_number());
            return;
        }
        send_sms_p(PSTR("Reset command accepted."), get_phone_number());
        reset_mcu();
    }
    else if(memcmp_P(ptr, PSTR("help;"), 5) == 0)
    {
        if((find_phone_in_phone_list(get_phone_number(), ADMIN_LIST) == 0))
        {
            send_sms_p(PSTR("Access denied"), get_phone_number());
            return;
        }
        send_sms_p(help_text, get_phone_number());
    }
    else if(memcmp_P(ptr, PSTR("helpget;"), 8) == 0)
    {
        if((find_phone_in_phone_list(get_phone_number(), ADMIN_LIST) == 0))
        {
            send_sms_p(PSTR("Access denied"), get_phone_number());
            return;
        }
        send_sms_p(help_get, get_phone_number());
    }
    else if(memcmp_P(ptr, PSTR("helpset;"), 8) == 0)
    {
        if((find_phone_in_phone_list(get_phone_number(), ADMIN_LIST) == 0))
        {
            send_sms_p(PSTR("Access denied"), get_phone_number());
            return;
        }
        send_sms_p(help_set, get_phone_number());
    }
    else if(memcmp_P(ptr, PSTR("report;"), 7) == 0)
    {
        if((find_phone_in_phone_list(get_phone_number(), ADMIN_LIST) == 0))
        {
            send_sms_p(PSTR("Access denied"), get_phone_number());
            return;
        }
        send_sms_report(get_phone_number());
    }
    else if(memcmp_P(ptr, PSTR("adminphones="), 12) == 0)
    {
        if( config.admin_mode == 0 || (find_phone_in_phone_list(get_phone_number(), ADMIN_LIST) == 1))
        {
            ptr+=12;
            ptr = set_phones(ptr, &config.admin_phone[0][0], TOTAL_ADMIN_NUMBER);
            config.admin_mode = 1;
            EEPROM_update();
            send_sms_p(PSTR("ok"), get_phone_number());
            softuart_puts_p(PSTR("set admin ok"));
            return;
        }
        else
        {
            send_sms_p(PSTR("Access denied"), get_phone_number());
            return;
        }

    }

}


//*******************************************************************************************************************

char* get_param(char *str, char *sms_text)
{
    if(memcmp_P(str, PSTR("adminphones;"), 12) == 0)
    {
        uint8_t n, i;

        str += 12;
        sms_text += sprintf_P(sms_text, PSTR("adminphones="));
        n=0;
        for(i=0; i<TOTAL_ADMIN_NUMBER; i++)
        {
            if(config.admin_phone[i][0] != '+')
                break;
            sms_text += sprintf_P(sms_text, PSTR("%s,"), &config.admin_phone[i][0]);
            n++; // количество напечатанных телефонов
        }
        if(n)
            sms_text--;
        sprintf_P(sms_text, PSTR(";"));
        return str;
    }

    else if(memcmp_P(str, PSTR("accept_calls;"), 13) == 0)
    {
        str += 13;
        sprintf_P(sms_text, PSTR("accept_calls=%d;"), config.accept_calls);
        return str;
    }

    else if(memcmp_P(str, PSTR("accept_sms;"), 11) == 0)
    {
        str += 11;
        sprintf_P(sms_text, PSTR("accept_sms=%d;"), config.accept_sms);
        return str;
    }

    else if(memcmp_P(str, PSTR("pause_relay;"), 12) == 0)
    {
        str += 12;
        sprintf_P(sms_text, PSTR("pause_relay=%u;"), config.report_temperature);
        return str;
    }
    return 0;
}

//*******************************************************************************************************************

char* set_param(char *ptr)
{
    if(memcmp_P(ptr, PSTR("accept_calls="), 13) == 0)
    {
        char temp;

        ptr+=13;
        if(*ptr == '0')
            temp = 0;
        else if(*ptr == '1')
            temp = 1;
        else
            return 0;
        if(*++ptr != ';')
            return 0;
        set_accept_calls(temp);
        EEPROM_update();
        ptr++;
        return ptr;
    }

    if(memcmp_P(ptr, PSTR("accept_sms="), 11) == 0)
    {
        char temp;

        ptr+=11;
        if(*ptr == '0')
            temp = 0;
        else if(*ptr == '1')
            temp = 1;
        else
            return 0;
        if(*++ptr != ';')
            return 0;
        config.accept_sms = temp;
        EEPROM_update();
        ptr++;
        return ptr;
    }

    if(memcmp_P(ptr, PSTR("report_t="), 9) == 0)
    {
        char temp;

        ptr+=9;
        if(*ptr == '0')
            temp = 0;
        else if(*ptr == '1')
            temp = 1;
        else
            return 0;
        if(*++ptr != ';')
            return 0;
        config.report_temperature = temp;
        EEPROM_update();
        ptr++;
        return ptr;
    }

    if(memcmp_P(ptr, PSTR("chapeka="), 8) == 0)
    {
        ptr+=8;
        if(*ptr == '0')
        {
            config.mode_gsm = GSM_ON_UTKINA;
            EEPROM_update();
        }
        else if(*ptr == '1')
        {
            config.mode_gsm = GSM_ON_CHAPEKA;
            EEPROM_update();
        }
        else
            return 0;
        ptr++;
        return ptr;
    }

    if(memcmp_P(ptr, PSTR("utkina="), 8) == 0)
    {
        ptr+=18;
        if(*ptr == '0')
        {
            config.mode_gsm = GSM_ON_CHAPEKA;
            EEPROM_update();
        }
        else if(*ptr == '1')
        {
            config.mode_gsm = GSM_ON_UTKINA;
            EEPROM_update();
        }
        else
            return 0;
        ptr++;
        return ptr;
    }

    if(memcmp_P(ptr, PSTR("pause_relay="), 12) == 0)
    {
        uint8_t interval;
        ptr+=12;
        if(isdigit(*ptr) == 0)
            return 0;
        interval = strtoul(ptr, &ptr, 10);
        if( interval>250 )
            return 0;
        if(*ptr != ';')
            return 0;
        set_pause_relay(interval);
        EEPROM_update();
        ptr++;
        return ptr;
    }
    return 0;
}

//*******************************************************************************************************************
// принимает список телефонов через запятую, адрес назначения и максимальный размер области назначения
static char* set_phones(char *phones, char *dest, char max_num)
{
    uint8_t n, i;
    char *ptr = phones;
    n=0;

    for(uint8_t j = 0; j < 5; j++)
    {
        config.admin_phone[j][0] = 0;
    }
    while(check_phone_string(ptr))
    {
        n++;
        if(n > max_num)
            return 0;
        ptr+=13;
        if(*ptr == ';')
            break;
        else if(*ptr == ',')
        {
            ptr++;
            continue;
        }
        else
            return 0;
    }
    ptr = phones;
    for(i=0; i<n; i++)
    {
        memcpy(&dest[i*14], ptr, 13);
        dest[i*14 + 13] = 0;
        ptr+=14;
    }
    memset(&dest[n*14], 0, 14*(max_num-n));
    config.admin_mode = 1;
    return ptr;
}
//*******************************************************************************************************************



























