#include "main.h"
#include "MOD_water_stats.h"
#include "MOD_valves.h"

//#define LIBCALL_ENABLEINTERRUPT
#include <EnableInterrupt.h>

#ifdef WITH_DS1307
#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#endif


void readCurChannelStats(waterStatsChanStorage_* waterStats)
{
   eeprom_read_bytes( sizeof(chanConf) * MAX_CHANNELS_ + sizeof(waterStatsChanStorage_) * __curChannel,
                      (byte*)(waterStats),
                      sizeof(waterStatsChanStorage_)); 
}

  /*
  Flow sensor Insterrupt Service Routine
   */
  void MOD_waterStats_::flowIncPulseCounter()
  { 
    // Increment the pulse counter
    flowPulseCount++;
    
    if (isAlertState() || __MOD_valves->stateMain)
      return;
      
    if (lastIncoherentPulseCountTime == 0 || ((millis() - lastIncoherentPulseCountTime) > 60000))
    {
       lastIncoherentPulseCountTime = millis();
       incoherentPulseCount = 0;
    }
      
    incoherentPulseCount++;
      
    if (incoherentPulseCount > 40)
    {
      __msg = F("water is flowing but\nvalves are closed!"); 
      setProgramAction(PRGM_STATE_ALERT);
    }    

  }


MOD_waterStats_::MOD_waterStats_() 
{   

    // The hall-effect flow sensor outputs approximately 71 pulses per second per
    // litre/minute of flow.
    flowSensorCalibrationFactor = 50.0f;

    incoherentPulseCount = 0;
    lastIncoherentPulseCountTime = 0;

    flowPulseCount = 0;
    flowStatsOldTime = 0;

    pinMode(WATER_FLOW_PIN, INPUT_PULLUP);
        
    reset();
       
    // The Hall-effect sensor is connected to pin 2 which uses interrupt 0.
    // Configured to trigger on a FALLING state change (transition from HIGH
    // state to LOW state)
    enableInterrupt(WATER_FLOW_PIN, globalFlowIncPulseCounter, FALLING);
}


bool MOD_waterStats_::reset() 
{
  waterFlow = WATER_STOPPED;
    
  flowStatsOldTime = 0;

  for (int ii = 0; ii< MAX_CHANNELS_; ++ii)
  {
    flowRate                  [ii] = 0.0f;
    totalMililitresSession    [ii] = 0;
    lastTotalMililitresSession[ii] = 0;               
  }
  return true;
}

bool MOD_waterStats_::start()
{
  waterFlow = WATER_STOPPED;
    
  // init water consumption statistics
  flowRate                  [__curChannel] = 0.0f;
  totalMililitresSession    [__curChannel] = 0;
  lastTotalMililitresSession[__curChannel] = 0;
  
  flowStatsOldTime = millis();
  return true;
}

void MOD_waterStats_::show(int _row)
{  
  char str[80];
  
  sprintf_P(str,PSTR("-Water used: %d ml"), totalMililitresSession[__curChannel]);
  LCD_PRINT(0,_row, str);
}

void MOD_waterStats_::printFlow() 
{
  char str[80];
  unsigned int frac;

  frac = (flowRate[__curChannel] - int(flowRate[__curChannel])) * 10;
  sprintf_P(str,PSTR("-Flow : %d,%d L/min"),int(flowRate[__curChannel]), frac);
  LCD_PRINT(0,2, str);
  
  sprintf_P(str,PSTR("-Water used: %d ml"), totalMililitresSession[__curChannel]);
  LCD_PRINT(0,3, str);
  
}

void MOD_waterStats_::saveSessionStats()
{
  waterStatsChanStorage_ waterStats;

  readCurChannelStats(&waterStats);

  #ifdef WITH_DS1307
  tmElements_t tm;
  if (RTC.read(tm))
  {      
    waterStats.wateringSession[waterStats.watSessionLogLine].mlUsed = totalMililitresSession    [__curChannel];
    waterStats.wateringSession[waterStats.watSessionLogLine].dateTime = makeTime(tm);

    if (++waterStats.watSessionLogLine >= STAT_LOG_SIZE)
      waterStats. watSessionLogLine = 0;
  }
  #endif
  
  waterStats.totalMililitres += totalMililitresSession    [__curChannel];
  ++waterStats.nbWaterings;
  
  // enregistrement des stats
  eeprom_write_bytes( sizeof(chanConf) * MAX_CHANNELS_ + sizeof(waterStatsChanStorage_) * __curChannel,
                      (const byte*)(&waterStats),
                      sizeof(waterStatsChanStorage_)); 
}



void MOD_waterStats_::readLogStats(char text[20][LCD_COLUMNS_+1], int& len, waterStatsChanStorage_ & waterStats) {
 
  tmElements_t tm;
  
  readCurChannelStats(&waterStats);

  if (waterStats.nbWaterings <= STAT_LOG_SIZE) // pas de rotation de log
  {
    len = waterStats.nbWaterings;
    for (int i = 0; i < len; ++i)
    {
      breakTime(waterStats.wateringSession[i].dateTime,tm);
      sprintf_P(text[i], PSTR("%02d/%02d %02d:%02d %d ml"), tm.Day, tm.Month, tm.Hour, tm.Minute, waterStats.wateringSession[i].mlUsed);
    }
  } else { // rotation de log
    len = STAT_LOG_SIZE;
    for (int i = 0; i < len; ++i)
    {
      int ii = (waterStats.watSessionLogLine + i) % STAT_LOG_SIZE;
            
      breakTime(waterStats.wateringSession[ii].dateTime,tm);
      sprintf_P(text[i], PSTR("%02d/%02d %02d:%02d %d ml"), tm.Day, tm.Month, tm.Hour, tm.Minute, waterStats.wateringSession[ii].mlUsed);
    }
  }

}








// Calcul des statistiques de consommation d'eau une fois une vanne ouverte
// ATTENTION : on part du principe qu'une seule vanne est ouverte à la fois.

  int MOD_waterStats_::calcFlow() 
  {
    unsigned long newTime = millis();

      // Disable the interrupt while calculating flow rate and sending the value to
      // the host
      disableInterrupt(WATER_FLOW_PIN);

      // Because this loop may not complete in exactly 1 second intervals we calculate
      // the number of milliseconds that have passed since the last execution and use
      // that to scale the output. We also apply the flowSensorCalibrationFactor to scale the output
      // based on the number of pulses per second per units of measure (litres/minute in
      // this case) coming from the sensor.
      
      flowRate[__curChannel] = ((1000.0f / (newTime - flowStatsOldTime)) * flowPulseCount) / flowSensorCalibrationFactor;
     
      // Reset the pulse counter so we can start incrementing again
      flowPulseCount = 0;
      
    // Enable the interrupt again now that we've finished sending output
    // The Hall-effect sensor is connected to pin 2 which uses interrupt 0.
    // Configured to trigger on a FALLING state change (transition from HIGH
    // state to LOW state)

    enableInterrupt(WATER_FLOW_PIN, globalFlowIncPulseCounter, FALLING);

      // Divide the flow rate in litres/minute by 60 to determine how many litres have
      // passed through the sensor in this 1 second interval, then multiply by 1000 to
      // convert to millilitres.
      unsigned int flowMilliLitres = (flowRate[__curChannel] / 60) * 1000;

      // Add the millilitres passed in this second to the cumulative total
      totalMililitresSession[__curChannel]  += flowMilliLitres;

      //TODO: remplacer
      //__channelConf[__curChannel].waterStatsStorage.totalMililitres += flowMilliLitres;

     // writeCurChannelConf();

      //DEBUG_PRINT("diff:");
      //DEBUG_PRINTLN(diffMillilitres);

      // détection d'incohérences
      
      int diffMillilitres = totalMililitresSession[__curChannel] - lastTotalMililitresSession[__curChannel];

      lastTotalMililitresSession[__curChannel] = totalMililitresSession[__curChannel];
       
      if (diffMillilitres > 0)
      {
        waterFlow = WATER_FLOWING;
        
        if ( totalMililitresSession[__curChannel] > __channelConf[__curChannel].maxMlPerSession )          
          waterFlow = WATER_OVERFLOW;
        
        // Note the time this processing pass was executed. Note that because we've
        // disabled interrupts the millis() function won't actually be incrementing right
        // at this point, but it will still return the value it was set to just before
        // interrupts went away.
        flowStatsOldTime = newTime;        
  
      } else {
        
        waterFlow = WATER_STOPPED;
        
        if (4000 < (newTime - flowStatsOldTime))
          waterFlow = WATER_BLOCKED;
        
      }

      unsigned int frac;

      #ifdef WITH_SERIAL  
            // Print the flow rate for this second in litres / minute
            Serial.print(F("Flow rate: "));
            Serial.print(int(flowRate[__curChannel]));  // Print the integer part of the variable
            Serial.print(F("."));             // Print the decimal point
            // Determine the fractional part. The 10 multiplier gives us 1 decimal place.
            frac = (flowRate[__curChannel] - int(flowRate[__curChannel])) * 10;
            Serial.print(frac, DEC) ;      // Print the fractional part of the variable
            Serial.print(F("L/min"));
            // Print the number of litres flowed in this second
            Serial.print(F("  Current Liquid Flowing: "));             // Output separator
            Serial.print(flowMilliLitres);
            Serial.print(F("mL/Sec"));
      
            // Print the cumulative total of litres flowed since starting
            Serial.print(F("  Output Liquid Quantity: "));             // Output separator
            Serial.print(lastTotalMililitresSession[__curChannel]);
            Serial.println(F("mL")); 
            delay(40);
      #endif

      
      return waterFlow;
  }
