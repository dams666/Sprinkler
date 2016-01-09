void initMoistureSensors()
{
  moistureSensorActivationPin = 32;  
     
  moistureSensorPins[0] = 52;
  if (NB_VALVES > 1) moistureSensorPins[1] = 50;
  if (NB_VALVES > 2) moistureSensorPins[2] = 48;
  if (NB_VALVES > 3) moistureSensorPins[3] = 46;
  if (NB_VALVES > 4) moistureSensorPins[4] = 44;
  if (NB_VALVES > 5) moistureSensorPins[5] = 42;

  pinMode(moistureSensorActivationPin, OUTPUT);

  for (int ii = 0; ii< NB_VALVES; ++ii)
  {
    pinMode(moistureSensorPins[ii], INPUT);
    
    moistureSensorState[ii]       = 0;
    prevMoistureSensorState[ii]   = 0;
  }
}
  
String getMoistureSensorsState()
{
    String str;

    for (int ii = 0; ii< NB_VALVES; ++ii)
    {
      str+= moistureSensorState[ii];
      str += " "; 
    }    
    return str;
  }

