#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED
#define F_CPU 7372800UL
//#define F_CPU 14745600UL

#define UNUSED1         B, 0
#define UNUSED2         B, 1
#define UNUSED3         B, 2
#define UNUSED4         B, 3
#define UNUSED5         C, 3
#define UNUSED6         C, 4

#define UNUSED8         D, 2
#define UNUSED9         D, 4


#define RED_LED	    	B, 5
#define GREEN_LED	    B, 4

#define PWR_GSM		    D, 7
#define RELAY_UTKINA	D, 6
#define RELAY_CHAPEKA   D, 5

#define PIN_CHECK_UTKINA    C, 0
#define PIN_CHECK_CHAPEKA   C, 1


#define BATTERY      5
#define INT_TEMP    8

#define PORT_MONITOR        PORTC

#define INPUT_UTKINA    0
#define INPUT_CHAPEKA   1

#define IS_VOLT_UTKINA  1
#define IS_VOLT_CHAPEKA 2
#define IS_VOLT_ALL     3
#define NOT_VOLT        0

#define GSM_ON_UTKINA   1
#define GSM_ON_CHAPEKA  2
#define GSM_REBOOT_LAN  3
#define GSM_OFF_LAN     4

#include <avr/io.h>
#include <avr/interrupt.h>
//#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <avr/eeprom.h>

#include "IO_Macros.h"
#include "softuart.h"
#include "timer.h"
#include "UART.h"
#include "port.h"
#include "GSM.h"
//#include "ADC.h"
#include "sms_parse.h"
#include "eeprom.h"
#include "app.h"




#endif // MAIN_H_INCLUDED
