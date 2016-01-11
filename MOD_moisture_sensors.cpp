#include "main.h"
#include "MOD_moisture_sensors.h"

MOD_moistureSensors_::MOD_moistureSensors_()
{
  activationPin = 32;  
  
  state     = new int[__nbChannels];
  prevState = new int[__nbChannels];
  pins      = new int[__nbChannels];

  memset (state, 0, sizeof(int) * __nbChannels);
  memset (prevState, 0, sizeof(int) * __nbChannels);
  memset (pins, 0, sizeof(int) * __nbChannels);
  
  pins[0] = 52;
  if (__nbChannels > 1) pins[1] = 50;
  if (__nbChannels > 2) pins[2] = 48;
  if (__nbChannels > 3) pins[3] = 46;
  if (__nbChannels > 4) pins[4] = 44;
  if (__nbChannels > 5) pins[5] = 42;

  pinMode(activationPin, OUTPUT);

  for (int ii = 0; ii< __nbChannels; ++ii)
    pinMode(pins[ii], INPUT);

}
  
String MOD_moistureSensors_::getState(bool newState)
{
    String str;
    str+="-Hum : [";
    
    for (int ii = 0; ii< __nbChannels; ++ii)
    {
      str+= newState ? state[ii] : prevState[ii];
      str += " "; 
    }    
    str+="]";
    return str;
  }

  void MOD_moistureSensors_::readValues()
  {
    for (int thisPin = 0; thisPin < __nbChannels; thisPin++)
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
  
