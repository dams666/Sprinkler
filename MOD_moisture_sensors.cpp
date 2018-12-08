#include "MOD_moisture_sensors.h"
#include "main.h"

#include <Adafruit_ADS1015.h>

#ifdef WITH_LOGGER
#include <SPI.h>
#include <SD.h>
#endif

MOD_moistureSensors_::MOD_moistureSensors_()
{
  memset (bits, 0, sizeof(int) * MAX_CHANNELS_);
  memset (volts, 0, sizeof(double) * MAX_CHANNELS_);
  memset (hum, 0, sizeof(int) * MAX_CHANNELS_);
  
  memset (state, 0, sizeof(bool) * MAX_CHANNELS_);
  memset (prevState, 0, sizeof(bool) * MAX_CHANNELS_);

  ads = new Adafruit_ADS1115 (0x4A);  /* Use this for the 16-bit version */

  // The ADC input range (or gain) can be changed via the following
  // functions, but be careful never to exceed VDD +0.3V max, or to
  // exceed the upper and lower limits if you adjust the input range!
  // Setting these values incorrectly may destroy your ADC!
  //                                                                ADS1015  ADS1115
  //                                                                -------  -------
  // ads->setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
     ads->setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  // ads->setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads->setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads->setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads->setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV

  pinMode(MOIST_SENS_PIN, OUTPUT);

  nextReadMillis = 0;
  reset();
}


bool MOD_moistureSensors_::reset()
{
  digitalWrite(MOIST_SENS_PIN, LOW);
  delay(50);
  return true;
}


bool MOD_moistureSensors_::start()
{
  digitalWrite(MOIST_SENS_PIN, HIGH);
  delay(50);
  ads->begin();
  return true;
}

void MOD_moistureSensors_::show(int _row)
{
    String str;
    
    str = F("-Hum : ");
    
    if (true) //__channelConf[__curChannel].active)
    {
       str += hum[__curChannel]; 
       str += F("% (");
       str += volts[__curChannel]; 
       str += F("V)   ");
    } else {
        str += F("- ");
    }
    
    LCD_PRINT(0,_row, str);    
  }

bool MOD_moistureSensors_::hasStateChanged()
{
  return state[__curChannel] != prevState[__curChannel];
}

void MOD_moistureSensors_::updateState()
{
  prevState[__curChannel] = state[__curChannel];
}

String MOD_moistureSensors_::getState(bool newState)
{
    String str;
    str+= F("-Hum : [");

    for (int ii = 0; ii< MAX_CHANNELS_; ++ii)
    { 
      if (__channelConf[ii].active)
      {
        str += newState ? state[ii] : prevState[ii];; 
      } else {
        str += F("-");
      }
    }    
    str+= F("]");
    return str;
  }

  void MOD_moistureSensors_::readValues()
  { 
    for (int thisPin = 0; thisPin < MAX_CHANNELS_; ++thisPin)
    {
      bits[thisPin] = 0;
      
      if (thisPin < 4)
        bits[thisPin]   = ads->readADC_SingleEnded(thisPin);

      volts[thisPin]  = bits[thisPin] * 0.000125;
      hum[thisPin]    = map(bits[thisPin],0,29200,0,100);

      state[thisPin] = hum[thisPin] < 80;
    }

    #ifdef WITH_LOGGER
    // ecriture dans le fichier de log
    if (millis() >= nextReadMillis)
    {
      nextReadMillis = millis() + 60000 * 5;

      __fileLogger = SD.open("logger.txt", FILE_WRITE);

      // if the file opened okay, write to it:
      if (__fileLogger)
      {
        __fileLogger.print(readTime());
        __fileLogger.print(" ");
        __fileLogger.print(volts[0]);
        __fileLogger.print(" ");
        __fileLogger.print(volts[1]);
        __fileLogger.print(" ");
        __fileLogger.print(volts[2]);
        __fileLogger.print(" ");
        __fileLogger.println(volts[3]);
        
        __fileLogger.close();
      }      
    }
    #endif
  }
  
