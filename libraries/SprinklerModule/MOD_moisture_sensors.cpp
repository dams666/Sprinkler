#include "MOD_moisture_sensors.h"

#include <Adafruit_ADS1015.h>
#include <TimeLib.h>

#if defined( WITH_SD_CARD )
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
	
  ads = new Adafruit_ADS1115 (ADS1115_I2C_ADDR);  /* Use this for the 16-bit version */
		
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

  #if defined( WITH_SD_CARD )
  SD.begin(4);
  #endif
  
  nextReadMillis = 0;
  stop();
}

MOD_moistureSensors_::~MOD_moistureSensors_()
{
	stop();
	delete ads;
}

bool MOD_moistureSensors_::stop()
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

void MOD_moistureSensors_::show(uint8_t channel, char* str)
{
  sprintf_P(str, PSTR("-Hum : %d%% (%d V)"), hum[channel], volts[channel]); 
}

bool MOD_moistureSensors_::getState(uint8_t channel) const
{
  return state[channel];
}

bool MOD_moistureSensors_::hasStateChanged(uint8_t channel) const 
{
  return state[channel] != prevState[channel];
}

/*
String MOD_moistureSensors_::getState(bool newState)
{
    String str;
    str+= F("-Hum : [");

    for (uint8_t ii = 0; ii< MAX_CHANNELS_; ++ii)
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
*/

  bool MOD_moistureSensors_::execute()
  { 
    for (uint8_t thisPin = 0; thisPin < MAX_CHANNELS_; ++thisPin)
    {
      bits[thisPin] = 0;
      
      if (thisPin < 4)
        bits[thisPin]   = ads->readADC_SingleEnded(thisPin);

      volts[thisPin]  = bits[thisPin] * 0.000125;
      hum[thisPin]    = map(bits[thisPin],0,29200,0,100);

      prevState[thisPin] = state[thisPin];
      state[thisPin] = hum[thisPin] < 80;
    }

	#if defined ( WITH_SERIAL ) || defined ( WITH_SD_CARD )
	// ecriture dans le fichier de log
    //if (millis() >= nextReadMillis)
    {
		char log[150];
		long val  = millis() / 1000;
		sprintf_P(log, PSTR("%d %02d:%02d %d %d %d %d"), elapsedDays(val), numberOfHours(val), numberOfMinutes(val), (int)(volts[0] * 100.0f), (int)(volts[1] * 100.0f), (int)(volts[2] * 100.0f), (int)(volts[3] * 100.0f));
      
		#if defined ( WITH_SERIAL )
		Serial.println(log);
		#endif

		#if defined( WITH_SD_CARD )
		//nextReadMillis = millis() + 60000 * 5;
		fileLogger = SD.open("logger.txt", FILE_WRITE);

		if (fileLogger)
		{
			fileLogger.println(log);
			fileLogger.close();
		}      
		#endif
    }
    #endif
    return true;
  }
  
