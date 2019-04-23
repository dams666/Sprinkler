#ifndef __MODULE_CONFIG_H__
#define __MODULE_CONFIG_H__

// definir une valeur entre 3 et 6
#define MAX_CHANNELS_ 3

//#define WITH_SERIAL
#undef WITH_SERIAL

//------------------------------------
// MOISTURE SENSORS MODULE
//------------------------------------

#define MOIST_SENS_PIN 3
#define ADS1115_I2C_ADDR 0x48

//#define WITH_SD_CARD
#undef WITH_SD_CARD

//------------------------------------
// VALVES MODULE
//------------------------------------

#define VALVE_OFF LOW
#define VALVE_ON HIGH

#define VALVE_M_PIN 6
#define VALVE_1_PIN 12
#define VALVE_2_PIN 11
#define VALVE_3_PIN 10
#define VALVE_4_PIN 9
#define VALVE_5_PIN 8
#define VALVE_6_PIN 7

#define MAX_VALVES_OPENED 1

//------------------------------------
// WATER STATS MODULE
//------------------------------------

#define WATER_FLOW_PIN 2
#define MAX_MILILITRES_PER_VALVE  50

#define STAT_LOG_SIZE 20

#endif
