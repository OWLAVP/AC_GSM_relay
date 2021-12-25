

#ifndef EEPROM_H_
#define EEPROM_H_


#define TOTAL_ADMIN_NUMBER      5
#define ADMIN_LIST					1

typedef struct
{
    uint32_t reset_count_mcu;
    uint8_t admin_mode;
    char admin_phone[TOTAL_ADMIN_NUMBER][14];
    uint8_t mode_gsm;
    uint8_t first_usage; // по величине этого параметра определяется чистота EEPROM
    uint8_t accept_calls;
    uint8_t accept_sms;
    uint8_t reset_not_reg;
    uint8_t interval_power_off_report_m;
    uint8_t interval_power_on_report_m;
    uint8_t report_temperature;
    uint8_t pause_relay;
} config_t;

extern config_t config;

void eeprom_read_config(uint8_t start);
void eeprom_save_config(void);
void EEPROM_update(void);

uint8_t get_interval_power_off_report(void);
void set_interval_power_off_report(uint8_t minute);
uint8_t get_interval_power_on_report(void);
void set_interval_power_on_report(uint8_t minute);
void set_mode_gsm(uint8_t on_off);
uint8_t get_mode_gsm(void);
void set_reset_count_mcu(uint32_t cnt);
uint32_t get_reset_count_mcu(void);
void inc_reset_count_mcu(void);
void set_report_temperature(uint8_t cnt);
uint8_t get_report_temperature(void);
void set_first_usage(uint8_t data);
void set_accept_calls(uint8_t on_off);
uint8_t get_accept_calls(void);
void set_accept_sms(uint8_t on_off);
uint8_t get_accept_sms(void);
void set_pause_relay(uint8_t seconds);
uint8_t get_pause_relay(void);
#endif /* EEPROM_H_ */


