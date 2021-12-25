
#include "main.h"
config_t config;
//config_t *p_config = &config;

static config_t *p_config = &config;
//*******************************************************************************************************************
// здесь обязательно нужен высокий уровень оптимизации, т.к. между записями EEMWE EEWE должно быть не более 4 тактов
void __attribute__((optimize("-O3"))) EEPROM_write(uint16_t address, uint8_t data)
{
    char tsreg;
    tsreg = SREG;
    cli();
    while(EECR & (1<<EEPE)); // Wait for completion of previous write
    EEAR = address; // Set up address and data registers
    EEDR = data;
    EECR = (1<<EEMPE); // Write logical one to EEMWE
    EECR = (1<<EEPE); // Start eeprom write by setting EEWE
    SREG = tsreg;
}
//void EEPROM_write(uint16_t uiAddress, unsigned char ucData)
//{
//    /* Wait for completion of previous write */
//    while(EECR & (1<<EEPE))
//        ;
//    /* Set up address and Data Registers */
//    EEAR = uiAddress;
//    EEDR = ucData;
//    /* Write logical one to EEMPE */
//    EECR |= (1<<EEMPE);
//    /* Start eeprom write by setting EEPE */
//    EECR |= (1<<EEPE);
//}
//*******************************************************************************************************************

uint8_t EEPROM_read(uint16_t address)
{
    while(EECR & (1<<EEPE)); // Wait for completion of previous write
    EEAR = address; // Set up address register
    EECR |= (1<<EERE); // Start eeprom read by writing EERE
    return EEDR; // Return data from data register
}
//*******************************************************************************************************************

void EEPROM_write_buf(char *buf, uint16_t len, uint16_t address)
{
    while(len--)
        EEPROM_write(address++, *buf++);
}

//*******************************************************************************************************************

void EEPROM_read_buf(char *buf, uint16_t len, uint16_t address)
{
    while(len--)
        *buf++ = EEPROM_read(address++);
}

//*******************************************************************************************************************

void eeprom_read_config(uint8_t start)
{
    eeprom_read_block((void*)&config, 0, sizeof(config_t));
    //softuart_puts_p(PSTR("eeprom\r\n"));
    //EEPROM_read_buf((char*)&config, sizeof(config_t), 0);
    if(p_config->first_usage != 0xAB)
    {
        softuart_puts_p(PSTR("eeprom_read_config\r\n"));
        memset(&config, 0, sizeof(config_t));
        p_config->interval_power_off_report_m = 1;
        p_config->interval_power_on_report_m = 2;
        p_config->accept_calls = 1;
        p_config->accept_sms = 1;
        p_config->mode_gsm = GSM_ON_UTKINA;
        p_config->first_usage = 0xAB;
        p_config->report_temperature = 1;
        p_config->pause_relay = 5;
        memcpy(p_config->admin_phone[0], "+380713318090", 13);
        //strcpy(p_config->admin_phone[0], "+380713318090");
        eeprom_save_config();
    }
    if(start)
    {
        //softuart_puts_p(PSTR("eeprom end\r\n"));
//        set_time_from_motion_s(config.time_from_motion_s);
    }
}

//*******************************************************************************************************************

void eeprom_save_config(void)
{
//    config.time_from_motion_s = get_time_from_motion_s();
    //EEPROM_write_buf((char*)&config, sizeof(config_t), 0);
    eeprom_write_block((void*)&config, 0, sizeof(config_t));
}

//*******************************************************************************************************************


//*******************************************************************************************************************

void EEPROM_update(void)
{
    eeprom_update_block((void*)&config, 0, sizeof(config_t));
}




//***********************************************
uint8_t get_interval_power_off_report(void)
{
    return p_config->interval_power_off_report_m;
}
void set_interval_power_off_report(uint8_t minute)
{
    p_config->interval_power_off_report_m = minute;
}
//***********************************************
uint8_t get_interval_power_on_report(void)
{
    return p_config->interval_power_on_report_m;
}
void set_interval_power_on_report(uint8_t minute)
{
    p_config->interval_power_on_report_m = minute;
}

//***********************************************
//******************************************************************
void set_accept_calls(uint8_t on_off)
{
    p_config->accept_calls = on_off;
}
uint8_t get_accept_calls(void)
{
    return p_config->accept_calls;
}
//**********************************************************************
void set_accept_sms(uint8_t on_off)
{
    p_config->accept_sms = on_off;
}
uint8_t get_accept_sms(void)
{
    return p_config->accept_sms;
}
//***********************************************
//**********************************************************************
void set_mode_gsm(uint8_t on_off)
{
    p_config->mode_gsm = on_off;
}
uint8_t get_mode_gsm(void)
{
    return p_config->mode_gsm;
}
//***********************************************

void set_reset_count_mcu(uint32_t cnt)
{
    p_config->reset_count_mcu = cnt;
}

uint32_t get_reset_count_mcu(void)
{
    return p_config->reset_count_mcu;
}

void inc_reset_count_mcu(void)
{
    config.reset_count_mcu++;
}
//***********************************************

void set_report_temperature(uint8_t cnt)
{
    p_config->report_temperature = cnt;
}
uint8_t get_report_temperature(void)
{
    return p_config->report_temperature;
}
//***********************************************
void set_first_usage(uint8_t data)
{
    p_config->first_usage = data;
}
//***********************************************

void set_pause_relay(uint8_t seconds)
{
    p_config->pause_relay = seconds;
}
uint8_t get_pause_relay(void)
{
    return p_config->pause_relay;
}
//***********************************************


