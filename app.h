#ifndef APP_H_INCLUDED
#define APP_H_INCLUDED

#define CHIP_TEMP_ERROR_OFFSET 	-45    	  // Offset calibration in degrees Celsius
#define CHIP_TEMP_NR_OF_SAMPLES	50       // To get a useful accuracy we have to take the average of 100 samples
//#define GAIN_FACTOR			0.075
//sensitivity




void on_off_relay();
void send_sms_report(char *phone);
void check_power(void);
void power_control(void);
void debug_report(void);
void init_internal_temperature(void);
int8_t get_filtered_temperature(void);
int16_t analog_read(uint8_t input);
void check_temperature(void);
void charge_bat(void);
//uint8_t level_battery(void);
void print_freeRam(void);
#endif // APP_H_INCLUDED
