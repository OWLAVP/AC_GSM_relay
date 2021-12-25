


#ifndef _GSM_H_
#define _GSM_H_

#define NET_BUF_SIZE	   320
#define TIME_CHECK_REG		300 //wait n seconds unsent sms and check reg. gsm

#define RESPON_OK			"OK"
#define RESPON_CLCC         "+CLCC:"
#define RESPON_SMS			"+CPMS:"
#define RESPON_CREG          "+CREG: 0,1"
#define RESPON_END			"\n"
#define RESPON_OK_GPRS		"+SAPBR: 1,1"
#define RESPON_OK_CSQ       "+CSQ:"
#define RESPON_OK_CMGL      "+CMGL:"
#define RESPON_ERROR	    "ERROR"
#define RESPON_CMSS      "+CMSS:"
#define RESPON_CMGS      "+CMGS:"

#define GSM_OK				1
#define GSM_CALL_ACTIVE		2   //	Состояние вызова - активное голосовое соединение.
#define GSM_CALL_HELD		3	//	Состояние вызова - удерживаемый.
#define GSM_CALL_END		4	//	Состояние вызова - разъединение.
#define GSM_CALL_OUT_DIAL	5   //	Состояние вызова - исходящий в режиме набора адреса.
#define GSM_CALL_OUT_BEEP	6	//	Состояние вызова - исходящий в режиме дозвона.
#define GSM_CALL_IN_BEEP	7	//	Состояние вызова - входящий  в режиме дозвона.
#define GSM_CALL_IN_WAIT	8	//	Состояние вызова - входящий  в режиме ожидания.
#define GSM_CALL_ERR		0


///////////// filename  ////////////////////
#define CHAPEKA_ON           "ch-on"
#define CHAPEKA_NOT          "ch-not"
#define UTKINA_ON            "ut-on" ///
#define UTKINA_NOT           "ut-not"
#define ENERGY_OK            "el-ok"
#define ALL_NOT              "all-not"
#define MENU                "menu"
#define REPORT_SEND         "r-send"
#define RST_LAN               "el-rst"
#define ALARM_TEMPERATURE    "temp"
char *get_phone_number(void);
//uint8_t hang_up_call(void);
uint8_t send_sms(char *str, char *phone);
uint8_t send_sms_p(const char *str, char *phone);
uint8_t check_phone_string(char *ptr);
uint8_t get_message_from_mdm(void);
//uint8_t mdm_wait_str(uint16_t time_to_wait_ms);
uint8_t get_sms(void);
char delete_all_sms(void);
//uint8_t mdm_get_battery_level(void);
uint8_t mdm_get_signal_strength(void);
uint8_t check_registration(void);
void incoming_call_processing(void);
void reset_mcu(void);
//char registration_status_ms(uint32_t time_to_wait_ms);
uint8_t setup_init();
//uint8_t save_sms_sim(char *_msg, char *_phone);
//uint8_t send_unsent_sms(void);
//uint8_t mdm_wait_cmd_ms(uint16_t time_to_wait_ms, const char* _cmd);
//uint8_t	call_status(void);
uint8_t call_to(char *phone, char *file_amr);
void alarm_play_file(char * file);
uint8_t get_error_code1();
uint16_t get_error_gsm();
void set_error_gsm(uint16_t _err);
//char* UCS2_to_string(char *s);
char* find_char(char *str, char ch);
char send_USSD(char * ussd);
void check_deposit(void);
void led_gsm(void);
void check_light(void);
void send_sms_admins(const char *_msg);
//void SMSdecodUCS2(char* dst, char* src, uint16_t len, uint16_t pos);
char* get_arr_dep(void);
//uint8_t play_amr(char * file_sim800, char repeat);
uint8_t map_s(uint16_t x, uint16_t in_min, uint16_t in_max, uint8_t out_min, uint8_t out_max);
#endif
