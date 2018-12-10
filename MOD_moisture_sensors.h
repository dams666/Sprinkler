#ifndef __MOISTURE_SENSOR_H__
#define __MOISTURE_SENSOR_H__

#include <arduino.h>
#include "module.h"
#include "config.h"

class Adafruit_ADS1115;
class MOD_moistureSensors_ : public Module
{
  public:

  uint16_t bits[MAX_CHANNELS_];
  uint16_t hum[MAX_CHANNELS_];
  double volts[MAX_CHANNELS_];
  
  bool state[MAX_CHANNELS_];             // current moistureSensorState of the machine
  bool prevState[MAX_CHANNELS_];          // previous MOD_moistureSensors.state of the machine
  
  protected:
  
   Adafruit_ADS1115 *ads;

  unsigned long nextReadMillis;
  
  public:
  
  MOD_moistureSensors_();

  bool reset();
  bool start();
  
  void readValues();

  bool hasStateChanged();
  void updateState();
  
  String getState(bool newState = true);
  void show(uint8_t);

};


#endif
