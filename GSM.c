#include "main.h"

//config_t config;
static char mdm_data[NET_BUF_SIZE];
static char rec_phone_number[14];
static uint8_t unsent_sms = 1;
static uint8_t registered_in_gsm_network = 0;
static uint8_t unread_sms = 0; //
static uint8_t call_from_user = 0;
static uint8_t incoming_call = 0;
static uint8_t error_code1 = 0;
static uint16_t error_gsm = 0;
static char arr_dep[9];
static int16_t deposit = 0;
//static char test_user[] = "+380713318090";

static char* gsm_poll_for_string(void);
static uint8_t call_status(void);
static uint8_t play_amr(char * file_sim800, char repeat);
static uint8_t hang_up_call(void);
static uint8_t mdm_wait_str(uint16_t time_to_wait_ms);
static uint8_t mdm_wait_prompt_ms(uint16_t time_to_wait_ms);
static uint8_t mdm_wait_cmd_ms(uint16_t time_to_wait_ms, const char* _answer);
static char registration_status_ms(uint32_t time_to_wait_ms);
//static uint8_t map_s(uint8_t x, uint8_t in_min, uint8_t in_max, uint8_t out_min, uint8_t out_max);
static uint8_t save_sms_sim(char *_msg, char *_phone);
static void wait_the_end_of_flow_from_mdm_ms(uint16_t time_to_wait_ms);
static uint8_t send_unsent_sms(void);
//static char * UCS2_to_string(char *s);
static unsigned char hex_to_char(char c);
static void SMSdecodUCS2(char* dst, char* src, uint16_t len, uint16_t pos);
static uint8_t len_to_char(char *p_src, uint8_t end_ch);
//*******************************************************************************************************************

static char* gsm_poll_for_string(void)
{
    static uint8_t state=0;
    static uint16_t i;
    static uint32_t time_stamp;
    unsigned char ch;

    if((get_time_s() - time_stamp) > 10)
    {
        if(state)
        {
            error_code1 = state;
            state = 0;
        }
    }
    if(is_queue_not_empty())
    {
        time_stamp = get_time_s();
        ch = get_byte_from_queue();
        switch(state)
        {
        case 0:
            if((ch != 0) && (ch != '\r') && (ch != '\n'))
            {
                mdm_data[0] = ch;
                i = 1;
                state = 3;
                if(ch == '>')
                {
                    mdm_data[1] = 0;
                    state = 0;
                    return mdm_data;
                }
            }
            break;
        case 3:
            mdm_data[i] = ch;
            i++;
            if(i>=NET_BUF_SIZE)
            {
                i=0;
                state = 0; // переполнение входного буфера
                break;
            }
            if(i>=2)
            {
                if((mdm_data[i-2] == '\r')&&(mdm_data[i-1] == '\n'))
                {
                    mdm_data[i-2] = 0;
                    state = 0;
                    return mdm_data;
                }
            }
            break;
        }
    }
    return 0;
}

// //*******************************************************************************************************************

static uint8_t call_status(void) 																														//	Аргументы функции:	отсутствуют.
{
    char *ptr;

    for(uint8_t i = 0; i < 5; i++)
    {
        if(i != 0)
            delay_ms(100);
        uart_flush();
        uart_send_str_p(PSTR("AT+CLCC\r\n"));
        if(mdm_wait_str(2000))
        {
            ptr = strstr_P(mdm_data, PSTR("+CLCC:"));
            if(ptr)
            {
                ptr = strchr(mdm_data, ','); // ищем первую запятую в строке
                if(ptr)
                {
                    ptr++;
                    ptr = strchr(ptr, ','); // ищем вторую запятую в строке
                    if(ptr)
                    {

                        if(isdigit(*++ptr))
                        {
                            switch( ptr[0]) 																													//	Получаем значение состояния вызова.
                            {
                            case '0':
                                return GSM_CALL_ACTIVE;
                                break;	//	Состояние вызова - активное голосовое соединение.
                            case '1':
                                return GSM_CALL_HELD;
                                break;  //	Состояние вызова - удерживаемый.
                            case '2':
                                return GSM_CALL_OUT_DIAL;
                                break;	//	Состояние вызова - исходящий в режиме набора адреса.
                            case '3':
                                return GSM_CALL_OUT_BEEP;
                                break;  //	Состояние вызова - исходящий в режиме дозвона.
                            case '4':
                                return GSM_CALL_IN_BEEP;
                                break;	//	Состояние вызова - входящий  в режиме дозвона.
                            case '5':
                                return GSM_CALL_IN_WAIT;
                                break;	//	Состояние вызова - входящий  в режиме ожидания.
                            case '7':
                                return GSM_CALL_END;
                                break;	//	Состояние вызова - разъединение.
                            default:
                                return GSM_CALL_ERR;
                                break;	//	Состояние текущего вызова
                            }
                        }
                        else continue;
                    }
                    else continue;
                }
                else continue;
            }

            ptr = strstr_P(mdm_data, PSTR("ERROR"));
            if(ptr)
                continue;

            ptr = strstr_P(mdm_data, PSTR("+DTMF:"));
            if(ptr)   // DTFM msg
            {
                ptr += 6;
                if(isdigit(*++ptr))
                {
                    switch( ptr[0]) 																													//	Получаем значение состояния вызова.
                    {
                    case '0':          // press 0
                        hang_up_call();
                        break;
                    case '1':           //press 1
                        set_mode_gsm(GSM_ON_CHAPEKA);
                        play_amr(CHAPEKA_ON,'0');
                        EEPROM_update();
                        break;
                    case '2':
                        set_mode_gsm(GSM_ON_UTKINA);
                        play_amr(UTKINA_ON,'0');
                        EEPROM_update();
                        break;
                    case '3':
                        play_amr(REPORT_SEND,'0');
                        delay_ms(2000);
                        hang_up_call();
                        delay_ms(1000);
                        send_sms_report(rec_phone_number);
                        break;
                    case '4':
                        play_amr(RST_LAN,'0');
                        delay_ms(3000);
                        set_mode_gsm(GSM_REBOOT_LAN);
                        break;
                    case '5':
                        break;
                    case '6':
                        break;
                    case '7':
                        break;
                    default:
                        break;	//	Состояние текущего вызова
                    }
                }
                return GSM_CALL_ACTIVE;
            }
        }
    }
    return 0;
}
////////////////////////////////////////////////////////////////
static uint8_t play_amr(char * file_sim800, char repeat)
{
    uart_flush();
    uart_send_str_p(PSTR("AT+CREC=5\r\n")); // stop play
    delay_ms(200);
    uart_flush();
    uart_send_str_p(PSTR("AT+CREC=4,\"C:\\User\\"));
    uart_send_str(file_sim800);
    uart_send_str_p(PSTR(".amr\",0,100,"));
    uart_send_byte(repeat);
    uart_send_str_p(PSTR("\r\n"));

    if(mdm_wait_cmd_ms(2000, (const char*)RESPON_OK) == 1)
    {
        return 1;
    }
    return 0;
}
////////////////////////////////////////////////////////////
//"AT+CREC=4,\"C:\\User\\14.amr\",1,100"
uint8_t call_to(char *phone, char *file_amr)
{
    uint8_t i = 0;
    uint32_t time_stamp_s;
    hang_up_call();
    if(registered_in_gsm_network == 0)
    {
        if(error_gsm < 0xFFFF)
        {
            error_gsm++;
        }
        return 0;
    }
    if(deposit < 1)
        return 0;
    while(i < 5)
    {
        delay_ms(1000);
        i++;
        sprintf_P(mdm_data, PSTR("ATD%s;\r\n"), phone);
        //uart_flush();
        uart_send_str(mdm_data);
        if(mdm_wait_cmd_ms(2000, (const char*)RESPON_OK))
        {
            delay_ms(200);
            softuart_puts_p(PSTR("CALL_OUT_DIAL ... " ));
            while(call_status()==GSM_CALL_OUT_DIAL) // Цикл выполняется пока набирается номер ...
            {
                delay_ms(500); //          Можно добавить код который будет выполняться в процессе набора номера             //
            }
        }
        else
        {
            continue;
        }
        delay_ms(200);
        if(call_status()==GSM_CALL_OUT_BEEP)
        {
            //          Ждём поднятия трубки на вызываемой стороне:                                       //
            while(call_status()==GSM_CALL_OUT_BEEP)
            {
                delay_ms(500);
            }
        }
        delay_ms(200);
        if(call_status()==GSM_CALL_ACTIVE)                   // Если соединение установлено (абонент ответил), то ...
        {
            play_amr(file_amr, '1');
            time_stamp_s = get_time_s() + 10;
            //          Ждём завершения активного голосового соединения:                                  //
            while((call_status()==GSM_CALL_ACTIVE) && (get_time_s() < time_stamp_s))        // Цикл выполняется пока установлено активное голосовое соединение ...
            {
                delay_ms(1000);
            }
            softuart_puts_p(PSTR( "Call ended" ));
            hang_up_call();
            return 1;
        }
        else                                                                                  // Иначе, если активное голосовое соединение не было установлено, то ...
        {
            softuart_puts_p(PSTR( "No answer!" ));
            continue;                                                  // Абонент не отвечает.
        }
    }
    return FALSE;
}
// //*******************************************************************************************************************
//


////////////////////////////////////////////////////////////////////////
static uint8_t hang_up_call(void)
{
    uint8_t i, rez;

    for(i=0; i<5; i++)
    {
        if(i!=0)
            delay_ms(200);
        uart_flush();
        uart_send_str_p(PSTR("ATH\r\n"));
        rez = mdm_wait_cmd_ms(2000, (const char*)RESPON_OK);
        if(rez)
            break;
    }
    return rez;
}
//
// //*******************************************************************************************************************
//
static uint8_t mdm_wait_str(uint16_t time_to_wait_ms)
{
    uint32_t time_stamp = get_time_ms() + time_to_wait_ms;
    while(get_message_from_mdm()==0)
    {
        if((get_time_ms() > time_stamp) )
        {
            softuart_puts_p(PSTR( "wait str time end" ));
            return 0;
        }
    }
    return 1;
}

// //*******************************************************************************************************************
//
static uint8_t mdm_wait_prompt_ms(uint16_t time_to_wait_ms)
{
    uint32_t time_stamp = get_time_ms() + time_to_wait_ms;
    char *ptr;

    while(1)
    {
        while(get_message_from_mdm()==0)
        {
            if((get_time_ms() > time_stamp))
                return 0;
        }

        if(mdm_data[0] == '>')
            return 1;

        ptr = strstr_P(mdm_data, PSTR("ERROR"));
        if(ptr)
            return 0;
    }
}
// //*******************************************************************************************************************

static uint8_t mdm_wait_cmd_ms(uint16_t time_to_wait_ms, const char* _answer)
{
    uint32_t time_stamp = get_time_ms() + time_to_wait_ms;
    char *ptr;

    while(1)
    {
        while(get_message_from_mdm()==0)
        {
            if((get_time_ms() > time_stamp))
            {
                softuart_puts_p(PSTR( "cmd time end" ));
                return 0;
            }
        }
        ptr = strstr(mdm_data, (const char*)_answer);
        //print_freeRam();
        if(ptr)
            return 1;
        ptr = strstr_P(mdm_data, PSTR("ERROR"));
        if(ptr)
            return 0;
    }
}

// //*******************************************************************************************************************
//
static char registration_status_ms(uint32_t time_to_wait_ms)
{
    uint32_t time_stamp = get_time_ms() + time_to_wait_ms;
    char *ptr;
    delay_ms(200);
    uart_flush();
    uart_send_str_p(PSTR("AT+CREG?\r\n"));
    while(1)
    {
        while(get_message_from_mdm()==0)
        {
            if(get_time_ms() > time_stamp)
                return 0;
        }
        ptr = strstr_P(mdm_data, PSTR("+CREG:")); // статус регистрации в сети
        if(ptr)
        {
            ptr = strchr(ptr, ',');
            if(ptr)
            {
                uint32_t n = strtoul(++ptr, 0, 10);
                if((n==1)||(n==5)) // регистрация в домашней сети, или роуминге
                {
                    registered_in_gsm_network = 1;
                    return 1;
                }
                else
                {
                    registered_in_gsm_network = 0;
                    return 0;
                }
            }
        }
    }
    return 0;
}
///////////////////////////////////
uint8_t check_registration(void)
{
    static uint32_t time_of_last_check = 0;
    static char cnt=0;

    if(get_time_s() > (time_of_last_check + TIME_CHECK_REG))
    {
        if(registration_status_ms(2000) == 0)
        {
            cnt++;
            softuart_puts_p(PSTR("not reg gsm"));
            if(cnt >= 5)
            {
//                softuart_puts_p(PSTR("reset_\r\n"));
//                config.reset_not_reg++;
                reset_mcu();
            }
        }
        else
        {
            cnt=0;
            time_of_last_check = get_time_s();
            return 1;
        }
        time_of_last_check = get_time_s();
        send_unsent_sms();
    }
    return 0;
}
//---------------------------------------------------
void check_deposit(void)
{
    static uint32_t last_check_balance = 0;
    if(get_time_m() > (last_check_balance + 1440))//1440 min = 24h
    {
        if(send_USSD("*101#") == 0)
        {
            last_check_balance += 60;
            return;
        }
        last_check_balance = get_time_m();
    }
}

// //*******************************************************************************************************************
//
uint8_t map_s(uint16_t x, uint16_t in_min, uint16_t in_max, uint8_t out_min, uint8_t out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

uint8_t mdm_get_signal_strength(void)
{
    char *ptr;
    unsigned long level;
    delay_ms(50);
    uart_flush();
    uart_send_str_p(PSTR("AT+CSQ\r"));
    if(mdm_wait_cmd_ms(2000, (const char*)RESPON_OK_CSQ) == 0)
    {
        return 0;
    }
    ptr = strchr(mdm_data, ':');
    if(!ptr)
    {
        return 0;
    }
    ptr+=2;
    if(isdigit(*ptr) == FALSE)
    {
        return 0;
    }
    level = strtoul(ptr, 0, 10);
    if(level>=32)
    {
        return 0;
    }
    else
    {
        return map_s(level, 2, 31, 0, 100);
    }
    return 0;
}

// //*******************************************************************************************************************
//
uint8_t send_sms(char *str, char *phone)
{
    char rez;
    if(registration_status_ms(2000) == 0)
    {
        if(error_gsm < 0xFFFF)
        {
            error_gsm++;
        }
        save_sms_sim(str, phone);
        return 0;
    }
    if(deposit < 1)
        return 0;
    delay_ms(200);
    sprintf_P(mdm_data, PSTR("AT+CMGS=\"%s\"\r\n"), phone);
    uart_flush();
    uart_send_str(mdm_data);
    if(mdm_wait_prompt_ms(1000) == 0)
    {
        save_sms_sim(str, phone);
        return 0;
    }
    uart_send_str(str);
    delay_ms(100);
    uart_send_byte(0x1A);
    rez = mdm_wait_cmd_ms(60000, (const char*)RESPON_CMGS);
    if(!rez)
    {
        save_sms_sim(str, phone);
        //softuart_puts_p(PSTR( "save sms"));
    }
    return rez;
}

static uint8_t save_sms_sim(char *_msg, char *_phone)
{

    uint8_t rez = 0;
    sprintf_P(mdm_data, PSTR("AT+CMGW=\"%s\"\r\n"), _phone);
    for(uint8_t i = 0; i < 10; i++)
    {
        if(i!=0)
            delay_ms(300);
        uart_flush();
        uart_send_str(mdm_data);
        if(mdm_wait_prompt_ms(3000) == FALSE)
        {
            continue;
        }
        uart_flush();
        uart_send_str(_msg);
        delay_ms(20);
        uart_send_byte(0x1A);
        rez = mdm_wait_cmd_ms(2000, (const char*)RESPON_OK);
        if(rez == 1)
        {
            unsent_sms = 1;
            break;
        }
    }
    return rez;
}

uint8_t send_sms_p(const char *str, char *phone)
{
    char rez;
    if(registration_status_ms(2000) == FALSE)
    {
        save_sms_sim((char*)str, phone);
        return FALSE;
    }
    if(deposit < 1)
        return 0;
    delay_ms(200);
    sprintf_P(mdm_data, PSTR("AT+CMGS=\"%s\"\r\n"), phone);
    uart_flush();
    uart_send_str(mdm_data);
    if(mdm_wait_prompt_ms(1000) == FALSE)
    {
        save_sms_sim((char *)str, phone);
        return FALSE;
    }
    uart_send_str_p((const char*)str);
    delay_ms(100);
    uart_send_byte(0x1A);
    rez = mdm_wait_cmd_ms(60000, (const char*)RESPON_CMGS);
    if(!rez)
    {
        save_sms_sim((char*)str, phone);
    }
    return rez;
}
//
// //*******************************************************************************************************************
//
void send_sms_admins(const char *_msg)
{
    uint8_t i = 0;
    if(deposit < 1)
        return;
    while(config.admin_phone[i][0]=='+' && i<TOTAL_ADMIN_NUMBER)
    {
        if(i != 0)
            delay_ms(1000);
        send_sms_p(_msg, &config.admin_phone[i][0]);
        i++;
    }
}
//****************************************************************************
void alarm_play_file(char * file)
{
    uint8_t i = 0;
    if(deposit < 1)
        return;
    while(config.admin_phone[i][0]=='+' && i<TOTAL_ADMIN_NUMBER)
    {
        if(i != 0)
            delay_ms(1000);
        call_to(&config.admin_phone[i][0], file);
        i++;
    }
}
//***************************************************
uint8_t get_message_from_mdm()
{
    char *ptr;
    ptr = gsm_poll_for_string();

    if(ptr)
    {
        //softuart_puts_ln(ptr);
        if(strstr_P(ptr, PSTR("+CMTI:"))) // пришла асинхронная индикация о принятой СМСке
        {
            unread_sms = 1;
            return 1;
        }

        else if(strstr_P(ptr, PSTR("+CLIP:")))   // пришла асинхронная индикация о звонке
        {
            ptr += 6;
            ptr = strchr(ptr, '+');
            //memcpy(phone_of_incomong_call, ptr, 13);
            if(find_phone_in_phone_list(ptr, ADMIN_LIST)) // ищем телефон в списке юзеров
            {
                call_from_user = 1;
                memcpy(rec_phone_number, ptr, 13); // сохраняем номер телефона, с которого произошел звонок
            }
//            if(memcmp(ptr, test_user, 13) == 0)
//            {
//                call_from_user = 1;
//                memcpy(rec_phone_number, ptr, 13);
//                softuart_puts_p(PSTR("test user ok"));
//            }
            incoming_call = 1; // вызов будет сброшен в основном цикле программы
            return 1;
        }

        else if(strstr_P(ptr, PSTR("+DTMF:")))   // DTFM msg
        {
            ptr += 6;
            if(isdigit(*++ptr))
            {
                switch( ptr[0]) 																													//	Получаем значение состояния вызова.
                {
                case '0':          // press 0
//                    play_amr(END_CALL,'0');
//                    delay_ms(3000);
                    hang_up_call();
                    break;
                case '1':           //press 1
                    set_mode_gsm(GSM_ON_CHAPEKA);
                    play_amr(CHAPEKA_ON,'0');
                    EEPROM_update();
                    break;
                case '2':
                    set_mode_gsm(GSM_ON_UTKINA);
                    play_amr(UTKINA_ON,'0');
                    EEPROM_update();
                    break;
                case '3':
                    if(deposit < 1)
                        return 0;
                    play_amr(REPORT_SEND,'0');
                    delay_ms(2000);
                    hang_up_call();
                    delay_ms(1000);
                    send_sms_report(rec_phone_number);
                    break;
                case '4':
                    play_amr(RST_LAN,'0');
                    delay_ms(3000);
                    //set_mode_gsm(GSM_REBOOT_LAN);
                    break;
                case '5':
                    break;
                case '6':
                    break;
                case '7':
                    break;
                default:
                    break;	//	Состояние текущего вызова
                }
            }
            return 1;
        }
        return 1;
    }
    return 0;
}
//
//
// //*******************************************************************************************************************
// // проверка строки, содержащей телефонный номер.
uint8_t check_phone_string(char *ptr)
{
    char i;

    if(*ptr++ != '+')
        return FALSE;
    for(i=0; i<12; i++)
    {
        if(!isdigit(*ptr++))
        {
            return FALSE;
        }
    }
    return TRUE;
}
//

static void wait_the_end_of_flow_from_mdm_ms(uint16_t time_to_wait_ms)
{
    uint32_t time_stamp = get_time_ms() + time_to_wait_ms;
    volatile char dummy;

    while(1)
    {
        if(get_time_ms() < time_stamp) // интервал после последнего принятого байта
            return;
        if(is_queue_not_empty())
        {
            dummy = get_byte_from_queue();
            dummy = dummy;
            time_stamp = get_time_ms() + time_to_wait_ms;
        }
    }
}
// //*******************************************************************************************************************
//

static uint8_t send_unsent_sms(void)
{
    if(unsent_sms == 0)
    {
        return 0;
    }
    uint8_t rez, i;
    char *ptr;
    unsigned long index = 0;

    delay_ms(100);
    uart_flush();
    uart_send_str_p(PSTR("AT+CMGL=\"STO UNSENT\"\r\n")); // запрашиваем список всех СМСок

    rez = mdm_wait_cmd_ms(5000, (const char*)RESPON_OK_CMGL);
    if(rez==1)
    {
        ptr = strchr(mdm_data, ':');
        if(ptr)
        {
            for(i=0; i<5; i++)
            {
                if(isdigit(*++ptr))
                    break;
            }
            if(i==5)
            {
                unsent_sms = 0;
                return 0;
            }

            index = strtoul(ptr, 0, 10);
            if((uint16_t)index>0xFFFF)
                return 0;
            if((uint16_t)index > 3) //max sms save sim
            {
                delete_all_sms();
            }
            if(index < 1)
            {
                unsent_sms = 0;
                return 0;
            }
        }
        else
        {
            unsent_sms = 0;
            return 0;
        }
        ptr = strchr(ptr, ','); // ищем первую запятую в строке
        if(ptr)
        {
            ptr = strchr(ptr, ','); // ищем вторую запятую в строке
            if(ptr)
            {
                ptr = strchr(ptr, '+');
                if(ptr)
                {
                    if(check_phone_string(ptr))
                    {
                        memcpy(rec_phone_number, ptr, 13);

                        rec_phone_number[13] = 0;
//                        softuart_puts_ln(rec_phone_number);
                    }
                }
            }
        }
        delay_ms(3000);
        sprintf_P(mdm_data, PSTR("AT+CMSS=%d,\"%s\"\r\n"), (uint16_t)index, rec_phone_number);
        uart_flush();
        uart_send_str(mdm_data);
        rez = mdm_wait_cmd_ms(60000, (const char*)RESPON_CMSS);
        if(rez==1)
        {
            for(i=0; i<3; i++)
            {
                delay_ms(100);
                sprintf_P(mdm_data, PSTR("AT+CMGD=%d\r\n"), (uint16_t)index); // удаляем обработанную СМСку
                softuart_puts_p(PSTR("del_sms_ind"));
                uart_flush();
                uart_send_str(mdm_data);
                rez = mdm_wait_cmd_ms(6000, (const char*)RESPON_OK);
                if(rez == TRUE)
                    return 1;
            }
        }
        else return 0;
    }
    else unsent_sms = 0;
    return 0;
}
//////////////////////////////////////////////////////////////////////
uint8_t get_sms(void)
{
    uint8_t rez, i;
    char *ptr;
    unsigned long index;

    if(unread_sms == 0)
    {
        //softuart_puts_ln("unread_sms == 0");
        return 0;
    }
    delay_ms(100);
    uart_flush();
    uart_send_str_p(PSTR("AT+CMGL=\"REC UNREAD\"\r\n")); // запрашиваем список всех СМСок

    while(1)
    {
        rez = mdm_wait_cmd_ms(5000, (const char*)RESPON_OK_CMGL);
        if(rez==1)
        {
            ptr = strchr(mdm_data, ':');
            if(ptr)
            {
                for(i=0; i<5; i++)
                {
                    if(isdigit(*++ptr))
                        break;
                }
                if(i==5)
                    continue;
                index = strtoul(ptr, 0, 10);
                if(index>0xFFFF)
                    continue;
            }
            else
                continue;
            ptr = strchr(ptr, ','); // ищем первую запятую в строке
            if(ptr)
            {
                ptr = strchr(ptr, ','); // ищем вторую запятую в строке
                if(ptr)
                {
                    ptr = strchr(ptr, '+');
                    if(ptr)
                    {
                        if(check_phone_string(ptr))
                        {
                            memcpy(rec_phone_number, ptr, 13);

                            rec_phone_number[13] = 0;
                            //softuart_puts_ln(rec_phone_number);
                        }
                        else
                            continue;
                    }
                }
                else
                    continue;
            }
            else
                continue;
            rez = mdm_wait_str(5000);
            if(rez==1)
            {
                wait_the_end_of_flow_from_mdm_ms(1000); // ждем окончания потока данных от модема, иными словами, flush
                process_sms_body(mdm_data);
                for(i=0; i<3; i++)
                {
                    delay_ms(100);
                    sprintf_P(mdm_data, PSTR("AT+CMGD=%d\r\n"), (uint16_t)index); // удаляем обработанную СМСку
                    uart_flush();
                    uart_send_str(mdm_data);
                    rez = mdm_wait_cmd_ms(10000, (const char*)RESPON_OK);
                    if(rez == TRUE)
                        break;
                }
                delay_ms(100);
                return 1;
            }
            else
            {
                delete_all_sms();
                unread_sms = 0;
                return 1;
            }
        }
        else
        {
            delete_all_sms(); // попытка обойти глюк модема
            unread_sms = 0;
            return 0;
        }
    }
}
//
// //*******************************************************************************************************************
//
char delete_all_sms(void)
{
    char rez;
    delay_ms(100);
    uart_flush();
    uart_send_str_p(PSTR("AT+CMGDA=\"DEL ALL\"\r\n"));
    rez = mdm_wait_cmd_ms(25000, (const char*)RESPON_OK);
    //  softuart_puts_p(PSTR("del_all_sms"));
    return rez;
}

//*******************************************************************************************************************
void incoming_call_processing(void)
{
    static uint32_t duration_call_s = 0;
//    static uint8_t flag_call = 0;
    if(incoming_call == 0)
    {
//        if(flag_call == 1 && ((duration_call_s + 40) < get_time_s()))
//        {
//            hang_up_call();
//            flag_call = 0;
//        }
        return;
    }
//    delay_ms(500);
//    flag_call = 1;
    incoming_call = 0;
    if(call_from_user)
    {
        //uart_flush();
        uart_send_str_p(PSTR("ATA\r\n"));
        delay_ms(1000);
        play_amr(MENU, '0');
        call_from_user = 0;
        duration_call_s = get_time_s() + 40;
        while(call_status() == GSM_CALL_ACTIVE)
        {
            delay_ms(3000);
            if(get_time_s() > duration_call_s)
            {
                hang_up_call();
                break;
            }
        }
    }
    else
    {
        hang_up_call();
    }
}
//////////////////////////////////
void reset_mcu(void)
{
    if(get_reset_count_mcu() < 10000000)
        inc_reset_count_mcu();
    EEPROM_update();
    DigitalWrite(PWR_GSM, HIGH);
    delay_s(3);
    DigitalWrite(PWR_GSM, LOW);
    cli(); // запрещаем прерывания и ждем перезагрузки по сторожевому таймеру
    while(1);
}

//////////////////////////////////////
uint8_t setup_init()
{
    uint8_t respons;
    uint32_t time_stamp = get_time_s() + 600; // 600 sec init modem

    for(;;)
    {
        for(;;)
        {
            uart_flush();
            uart_send_str_p(PSTR("AT\r\n"));
            if ((respons = mdm_wait_cmd_ms(1000, (const char*)RESPON_OK)))
            {
                break;
            }
            if((get_time_s() > time_stamp))
            {
                reset_mcu();
            }
            delay_ms(1000);
        }
        delay_ms(500);
        uart_flush();
        uart_send_str_p(PSTR("ATE0\r\n"));
        delay_ms(2000);
//        uart_send_str_p(PSTR("AT+CSCS=\"HEX\"\r\n"));
//        delay_ms(1000);
        uart_flush();
        uart_send_str_p(PSTR("AT+CMGF=1\r\n"));
        if ((respons = mdm_wait_cmd_ms(3000, (const char*)RESPON_OK)))
        {
            delay_ms(500);
            uart_flush();
            uart_send_str_p(PSTR("AT+CNMI=2,1,0,1,0\r\n"));
            if ((respons = mdm_wait_cmd_ms(3000, (const char*)RESPON_OK)))
            {
                delay_ms(500);
//                uart_flush();
//                uart_send_str_p(PSTR("AT+CPMS=\"SM\",\"SM\",\"SM\"\r\n"));
//                delay_ms(2000);
                uart_flush();
                uart_send_str_p(PSTR("AT+CLIP=1\r\n"));
                if ((respons = mdm_wait_cmd_ms(3000, (const char*)RESPON_OK)))
                {
                    delay_ms(500);
                    uart_flush();
                    uart_send_str_p(PSTR("AT+CPMS=\"SM\",\"SM\",\"SM\"\r\n"));
                    if ((respons = mdm_wait_cmd_ms(3000, (const char*)RESPON_OK)))
                    {
                        delay_ms(500);
                        uart_flush();
                        uart_send_str_p(PSTR("AT+DDET=1\r\n"));
                        if ((respons = mdm_wait_cmd_ms(3000, (const char*)RESPON_OK)))
                        {
                            break;
                        }
                        else continue;
                    }
                    else continue;
                }
                else continue;
            }
            else continue;
        }
        else continue;
    }
    registration_status_ms(2000);
    delay_s(3);
    return respons;
}
// //*******************************************************************************************************************
//
//uint8_t mdm_get_battery_level(void)
//{
//    char *ptr;
//    char rez;
//    uint32_t level;
//    for(uint8_t i = 0; i < 5; i++)
//    {
//        delay_ms(100);
//        uart_flush();
//        uart_send_str_p(PSTR("AT+CBC\r\n"));
//        rez = mdm_wait_str(2000);
//        if(rez==FALSE)
//            return 0xFF;
//        ptr = strstr_P(mdm_data, PSTR("+CBC:"));
//        if(ptr==FALSE)
//            continue;
//        ptr = strchr(ptr, ':');
//        if(!ptr)
//            return 0xFF;
//        ptr = strchr(ptr, ',');
//        if(!ptr)
//            return 0xFF;
//        ptr++;
//        if(isdigit(*ptr) == FALSE)
//            return 0xFF;
//        level = strtoul(ptr, 0, 10);
//        if(level<=0xFF)
//            return (unsigned char)level;
//        else
//            return 0xFF;
//    }
//    return 0xFF;
//}
//
// //*******************************************************************************************************************

char send_USSD(char * ussd)
{
    uint32_t timeout = 0;
    char *ptr;
    uint8_t i;
    char temp_ussd[93];
    for(i = 0; i < 3; i++)
    {
        if(i != 0)
            delay_ms(500);
        uart_flush();
        uart_send_str_p(PSTR("AT+CUSD=1,\""));
        uart_send_str((char *)ussd);
        uart_send_str_p(PSTR("\"\r\n"));
        if(mdm_wait_cmd_ms(10000, (const char*)RESPON_OK) == 1)
        {
            timeout = get_time_s() + 10;
            softuart_puts_ln("ussd send ok");
            break;
        }
    }
    if(i == 3)
        return 0;


    while(get_time_s() < timeout)
    {
        ptr = gsm_poll_for_string();
        if(ptr)
        {
            if(strstr(ptr, "+CUSD:")) //
            {
                uint8_t cnt_ch;
                ptr += 6;
                ptr = strchr(ptr, '"');
                if(!ptr)
                    return 0;
                ptr++;
                //strncpy(temp_ussd, ptr, 92);
                memmove(temp_ussd, ptr, 92);
                SMSdecodUCS2(ptr, temp_ussd, 48, 0);
                //ptr = UCS2_to_string(ptr);
                ptr = strchr(ptr, ' ');
                if(ptr)
                {
                    ptr++;
                    ptr = strchr(ptr, ' ');
                    if(ptr)
                    {
                        ptr++;
                        cnt_ch = len_to_char(ptr, ' ');
                    }
                    else
                    {
                        //strcpy(arr_dep, "err");
                        memcpy(arr_dep, "err", 3);
                        return 0;
                    }
                }
                else
                {
                    //strcpy(arr_dep, "err");
                    memcpy(arr_dep, "err", 3);
                    return 0;
                }
                if(cnt_ch > 7)
                {
                    //strcpy(arr_dep, "err");
                    memcpy(arr_dep, "err", 3);
                    return 0;
                }
                //strncpy(arr_dep, ptr, cnt_ch);
                memmove(arr_dep, ptr, cnt_ch);
                ptr = strtok(ptr, ".");
                if(!ptr)
                {
                    return 0;
                }
                softuart_puts_ln(arr_dep);
                deposit = atoi(ptr);
                if(deposit <= 0)
                {
                    return 0;
                }
                else if(deposit < 10)
                {

                    softuart_puts_p(PSTR( "Deposit < 10 rub" ));
                    send_sms_admins(PSTR( "Deposit < 10 RUB" ));
                }
            }
            return 1;
        }
    }

    return 0;
}
////////////////
uint8_t get_error_code1()
{
    return error_code1;
}
////////////////
uint16_t get_error_gsm()
{
    return get_val(error_gsm);
}

////////////////////////////////////
void set_error_gsm(uint16_t _err)
{
    set_val(error_gsm, _err);
}


/////////////////////////////
static unsigned char hex_to_char(char c)
{
    uint8_t i = (uint8_t)c;
    if ( (i>=0x30) && (i<=0x39) )
    {
        return i-0x30;
    }
    else if ( (i>=0x41) && (i<=0x46) )
    {
        return i-0x37;
    }
    else if ( (i>=0x61) && (i<=0x66) )
    {
        return i-0x57;
    }
    else
    {
        return      0;
    }
}

//static char * UCS2_to_string(char *s)                         // Функция декодирования UCS2 строки
//{
//    char answer[50];
//    char c[5] = "";                            // Массив для хранения результата
//    uint8_t j = strlen(s);
//    for (int i = 0; i < (j - 3); i += 4)         // Перебираем по 4 символа кодировки
//    {
//        unsigned long code = (((unsigned int)hex_to_char(s[i])) << 12) +    // Получаем UNICODE-код символа из HEX представления
//                             (((unsigned int)hex_to_char(s[i + 1])) << 8) +
//                             (((unsigned int)hex_to_char(s[i + 2])) << 4) +
//                             ((unsigned int)hex_to_char(s[i + 3]));
//        if (code <= 0x7F)       // Теперь в соответствии с количеством байт формируем символ
//        {
//            c[0] = (char)code;
//            c[1] = 0;                                       // Не забываем про завершающий ноль
//        }
//        else if (code <= 0x7FF)
//        {
//            c[0] = (char)(0xC0 | (code >> 6));
//            c[1] = (char)(0x80 | (code & 0x3F));
//            c[2] = 0;
//        }
//        else if (code <= 0xFFFF)
//        {
//            c[0] = (char)(0xE0 | (code >> 12));
//            c[1] = (char)(0x80 | ((code >> 6) & 0x3F));
//            c[2] = (char)(0x80 | (code & 0x3F));
//            c[3] = 0;
//        }
//        else if (code <= 0x1FFFFF)
//        {
//            c[0] = (char)(0xE0 | (code >> 18));
//            c[1] = (char)(0xE0 | ((code >> 12) & 0x3F));
//            c[2] = (char)(0x80 | ((code >> 6) & 0x3F));
//            c[3] = (char)(0x80 | (code & 0x3F));
//            c[4] = 0;
//        }
//        strcat(answer, (char *)c);        // Добавляем полученный символ к результату
//    }
//    //answer[strlen(answer)]='\0';
//    strcpy(s, answer);
//    //softuart_puts_ln(temp_ussd);
//    return s;
//
//}


//******************************************************************************************
void led_gsm(void)
{
    static uint8_t j = 0;
    static uint8_t temp_flag = 0;

    if(temp_flag == 0 && registered_in_gsm_network == 1)
    {
        DigitalWrite(GREEN_LED, HIGH);
        temp_flag = 1;
    }
    else if(registered_in_gsm_network == 0)
    {
        j++;
        temp_flag = 0;
        if(j > 50)
        {
            DigitalLevelToggle(GREEN_LED);
            j = 0;
        }
    }
}
//******************************************************************************************

//*******************************************************************************************
char *get_phone_number(void)
{
    return rec_phone_number;
}

//*******************************************************************************************
static void SMSdecodUCS2(char* dst, char* src, uint16_t len, uint16_t pos)
{
    uint8_t  byteThis = 0;
    uint8_t  byteNext = 0;
    uint16_t numIn    = 0;
    uint16_t numOut   = 0;
    len*=2;
    //
    while(numIn<len)
    {
        byteThis = hex_to_char(src[pos+numIn])*16 + hex_to_char(src[pos+numIn+1]);
        numIn+=2;									//	Читаем значение очередного байта в переменную byteThis.
        byteNext = hex_to_char(src[pos+numIn])*16 + hex_to_char(src[pos+numIn+1]);
        numIn+=2;									//	Читаем значение следующего байта в переменную byteNext.
        if(byteThis==0x00)
        {
            dst[numOut]=byteNext;
            numOut++;
        }
        else											//  Символы латинницы
            if(byteNext==0x01)
            {
                dst[numOut]=0xD0;
                numOut++;
                dst[numOut]=byteNext+0x80;
                numOut++;
            }
            else											//	Симол  'Ё'       - 04 01          =>  208 129
                if(byteNext==0x51)
                {
                    dst[numOut]=0xD1;
                    numOut++;
                    dst[numOut]=byteNext+0x40;
                    numOut++;
                }
                else											//	Симол  'ё'       - 04 81          =>  209 145
                    if(byteNext< 0x40)
                    {
                        dst[numOut]=0xD0;
                        numOut++;
                        dst[numOut]=byteNext+0x80;
                        numOut++;
                    }
                    else											//	Симолы 'А-Я,а-п' - 04 16 - 04 63  =>  208 144 - 208 191
                    {
                        dst[numOut]=0xD1;    //	Симолы 'р-я'     - 04 64 - 04 79  =>  209 128 - 209 143
                        numOut++;
                        dst[numOut]=byteNext+0x40;
                        numOut++;
                    }
    }
    dst[numOut]=0;																									//
    //
    //
}

//*******************************************************************************************
char* get_arr_dep(void)
{
    return arr_dep;
}
//
//*******************************************************************************************

static uint8_t len_to_char(char *p_src, uint8_t end_ch)
{
    uint8_t i=0;

    for( i = 0; i < strlen(p_src) && p_src[i]!=end_ch; i++) {;}

    return i;
}

