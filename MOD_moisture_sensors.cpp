#include "main.h"
#include "MOD_moisture_sensors.h"

MOD_moistureSensors_::MOD_moistureSensors_()
{
  activationPin = 36;  
  
  state     = new int[MAX_CHANNELS_];
  prevState = new int[MAX_CHANNELS_];
  pins      = new int[MAX_CHANNELS_];

  memset (state, 0, sizeof(int) * MAX_CHANNELS_);
  memset (prevState, 0, sizeof(int) * MAX_CHANNELS_);
  memset (pins, 0, sizeof(int) * MAX_CHANNELS_);
  
  pins[0] = 52;
  pins[1] = 50;
  pins[2] = 48;
  pins[3] = 46;
  pins[4] = 44;
  pins[5] = 42;

  pinMode(activationPin, OUTPUT);

  for (int ii = 0; ii< MAX_CHANNELS_; ++ii)
    pinMode(pins[ii], INPUT);

  setEnabled(false);
}
  
String MOD_moistureSensors_::getState(bool newState)
{
    String str;
    str+="-Hum : [";

    for (int ii = 0; ii< MAX_CHANNELS_; ++ii)
    { 
      if (__channelConfig[ii].active)
      {
        str += newState ? state[ii] : prevState[ii];; 
      } else {
        str += "-";
      }
    }    
    str+="]";
    return str;
  }

  void MOD_moistureSensors_::readValues()
  {
    for (int thisPin = 0; thisPin < MAX_CHANNELS_; thisPin++)
    {
      state[thisPin] = digitalRead(pins[thisPin]);
    }
  }

  void MOD_moistureSensors_::setEnabled(bool enabled)
  {
    if (enabled)
    {
      digitalWrite(activationPin, HIGH);
    } else {
      digitalWrite(activationPin, LOW);
    }
    delay(50);
  }
  
