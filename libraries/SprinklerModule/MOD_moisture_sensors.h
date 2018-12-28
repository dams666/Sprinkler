#ifndef __MOISTURE_SENSOR_H__
#define __MOISTURE_SENSOR_H__

#include <arduino.h>
#include "module.h"
#include "ModuleConfig.h"

#if defined( WITH_SD_CARD )
#include <SD.h>
#endif

class Adafruit_ADS1115;

class MOD_moistureSensors_ : public Module
{
  protected:
  
  int bits[MAX_CHANNELS_];
  uint16_t hum[MAX_CHANNELS_];
  double volts[MAX_CHANNELS_];
  
  bool state[MAX_CHANNELS_];             // current moistureSensorState of the machine
  bool prevState[MAX_CHANNELS_];          // previous MOD_moistureSensors.state of the machine
    
   Adafruit_ADS1115 *ads;

  unsigned long nextReadMillis;
  
  #if defined( WITH_SD_CARD )
  SDLib::File	fileLogger;
  #endif
  
  public:
  
  MOD_moistureSensors_();
  ~MOD_moistureSensors_();
  
  bool stop();
  bool start();
  
  bool execute();

  bool getState(uint8_t) const;
  bool hasStateChanged(uint8_t) const ;
  
  //String getState(bool newState = true);
  void show(uint8_t, char*);

};


#endif
