#ifndef __MOISTURE_SENSOR_H__
#define __MOISTURE_SENSOR_H__

#include <arduino.h>
#include "module.h"

class Adafruit_ADS1115;
class MOD_moistureSensors_ : public Module
{
  public:

  int *bits;
  int *hum;
  double *volts;
  
  int *state;             // current moistureSensorState of the machine
  int *prevState;          // previous MOD_moistureSensors.state of the machine
  
  protected:
  
  int activationPin;

   Adafruit_ADS1115 *ads;

  unsigned long nextReadMillis;
  
  public:
  
  MOD_moistureSensors_();

  void reset();

  void readValues();

  bool hasStateChanged();
  void updateState();
  
  String getState(bool newState = true);
  void show(int);
  void setEnabled(bool);
};


#endif
