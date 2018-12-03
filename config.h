#ifndef __CONFIG_H__
#define __CONFIG_H__

#define MAX_CHANNELS_ 6
#define SLEEPING_DURATION_        10000

//#define WITH_SERIAL
#undef WITH_SERIAL
//#define WITH_DS1307
#undef WITH_DS1307
#undef WITH_LOGGER

#define BUZZER_PIN LED_BUILTIN

//------------------------------------
// GUI
//------------------------------------

#define LCD_I2C_ADDR 0x3F
#define RC_PIN 5

//------------------------------------
// MOISTURE SENSORS MODULE
//------------------------------------

#define MOIST_SENS_PIN 3

//------------------------------------
// VALVES MODULE
//------------------------------------

#define VALVE_M_PIN 6
#define VALVE_1_PIN 7
#define VALVE_2_PIN 8
#define VALVE_3_PIN 9
#define VALVE_4_PIN 10
#define VALVE_5_PIN 11
#define VALVE_6_PIN 12

#define MAX_VALVES_OPENED 1

//------------------------------------
// WATER STATS MODULE
//------------------------------------

#define WATER_FLOW_PIN 2
#define MAX_MILILITRES_PER_VALVE  50

#endif
