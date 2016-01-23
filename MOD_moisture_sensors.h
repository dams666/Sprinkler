#ifndef __MOISTURE_SENSOR_H__
#define __MOISTURE_SENSOR_H__

#include <arduino.h>
#include "module.h"

class MOD_moistureSensors_ : public Module
{
  public:
  
  int *state;             // current moistureSensorState of the machine
  int *prevState;          // previous MOD_moistureSensors.state of the machine
  
  protected:
  
  int activationPin;

  int *pins;

  public:
  
  MOD_moistureSensors_();

  void reset();

  void readValues();
  
  String getState(bool newState = true);

  void setEnabled(bool);
};


#endif
