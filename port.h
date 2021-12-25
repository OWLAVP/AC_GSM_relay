

#ifndef PORT_H_
#define PORT_H_


#define FRONT_SENSOR_MASK (1<<0)
#define INSIDE_1_SENSOR_MASK  (1<<1) //PD4, PD5
#define INSIDE_2_SENSOR_MASK (1<<2)
#define SIDE_SENSOR_MASK  (1<<3) //PD4, PD5

void port_init(void);

void check_volt_input(void);
uint8_t get_flag_input_voltage(void);

#endif /* PORT_H_ */
