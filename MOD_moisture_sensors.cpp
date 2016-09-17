#include "MOD_moisture_sensors.h"
#include "main.h"

#include <Adafruit_ADS1015.h>
#include <SPI.h>
#include <SD.h>

MOD_moistureSensors_::MOD_moistureSensors_()
{
  activationPin = 36;  

  bits      = new int[MAX_CHANNELS_];
  hum       = new int[MAX_CHANNELS_];
  volts     = new double[MAX_CHANNELS_];
  
  state     = new int[MAX_CHANNELS_];
  prevState = new int[MAX_CHANNELS_];

  memset (bits, 0, sizeof(int) * MAX_CHANNELS_);
  memset (volts, 0, sizeof(double) * MAX_CHANNELS_);
  memset (hum, 0, sizeof(int) * MAX_CHANNELS_);
  
  memset (state, 0, sizeof(int) * MAX_CHANNELS_);
  memset (prevState, 0, sizeof(int) * MAX_CHANNELS_);

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

  pinMode(activationPin, OUTPUT);

  nextReadMillis = 0;
  setEnabled(false);
}


void MOD_moistureSensors_::reset()
{
  setEnabled(false);
}

void MOD_moistureSensors_::show(int _row)
{
    String str;
    
    str = "-Hum : ";
    
    if (true) //__channelStorage[__curChannel].active)
    {
       str += hum[__curChannel]; 
       str += "% (";
       str += volts[__curChannel]; 
       str += "V)   ";
    } else {
        str += "- ";
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
    str+="-Hum : [";

    for (int ii = 0; ii< MAX_CHANNELS_; ++ii)
    { 
      if (__channelStorage[ii].active)
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
      if (thisPin < 4)
      {
        bits[thisPin]   = ads->readADC_SingleEnded(thisPin);
      } else {
        bits[thisPin] = 0;
      }
      volts[thisPin]  = bits[thisPin] * 0.000125;
      hum[thisPin]    = map(bits[thisPin],0,29200,0,100);

      state[thisPin] = hum[thisPin] < 80;
    }
    
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
  }
  
  void MOD_moistureSensors_::setEnabled(bool enabled)
  {
    if (enabled)
    {
      digitalWrite(activationPin, HIGH);
      delay(50);
      ads->begin();
      
    } else {
      digitalWrite(activationPin, LOW);
      delay(50);
    }

  }
  
