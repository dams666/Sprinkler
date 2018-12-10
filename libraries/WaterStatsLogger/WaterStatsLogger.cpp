
#include <Arduino.h>
#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#include <Eepromutil.h>

#include "WaterStatsLogger.h"

waterStatsLogger::waterStatsLogger(int addr)
{
  this->startAddr = addr;
} 

void waterStatsLogger::saveSessionStats(uint16_t totalMililitres)
{
  // lecture des stats
  eeprom_read_bytes( startAddr,(byte*)(&this->waterStats), sizeof(waterStatsChanStorage_)); 

  tmElements_t tm;
  if (RTC.read(tm))
  {      
    waterStats.wateringSession[waterStats.watSessionLogLine].mlUsed = totalMililitres;
    waterStats.wateringSession[waterStats.watSessionLogLine].dateTime = makeTime(tm);

    if (++waterStats.watSessionLogLine >= STAT_LOG_SIZE)
      waterStats. watSessionLogLine = 0;
  }
  
  waterStats.totalMililitres += totalMililitres;
  ++waterStats.nbWaterings;
  
  // enregistrement des stats
  /*
  eeprom_write_bytes( sizeof(chanConf) * MAX_CHANNELS_ + sizeof(waterStatsChanStorage_) * __curChannel,
                      (const byte*)(&waterStats),
                      sizeof(waterStatsChanStorage_)); 
*/

  eeprom_write_bytes(startAddr, (const byte*)(&this->waterStats), sizeof(waterStatsChanStorage_)); 
}


void waterStatsLogger::readLogStats(char** text, uint8_t& len, unsigned long& totalMililitres, uint16_t&  nbWaterings)
{
  tmElements_t tm;
  uint8_t i;
  
  for (i=0; i< STAT_LOG_SIZE; ++i) {
	text[i] = '\0';
  }
  
  // lecture des stats
  eeprom_read_bytes( startAddr,(byte*)(&this->waterStats), sizeof(waterStatsChanStorage_));
  
  nbWaterings = waterStats.nbWaterings;
  totalMililitres = waterStats.totalMililitres;

  if (waterStats.nbWaterings <= STAT_LOG_SIZE) // pas de rotation de log
  {
    len = waterStats.nbWaterings;
    for (i = 0; i < len; ++i)
    {
      breakTime(waterStats.wateringSession[i].dateTime,tm);
      sprintf_P(text[i], PSTR("-%02d/%02d %02d:%02d %d ml"), tm.Day, tm.Month, tm.Hour, tm.Minute, waterStats.wateringSession[i].mlUsed);
    }
  } else { // rotation de log
    len = STAT_LOG_SIZE;
    for (i = 0; i < len; ++i)
    {
      uint8_t ii = (waterStats.watSessionLogLine + i) % STAT_LOG_SIZE;
            
      breakTime(waterStats.wateringSession[ii].dateTime,tm);
      sprintf_P(text[i], PSTR("-%02d/%02d %02d:%02d %d ml"), tm.Day, tm.Month, tm.Hour, tm.Minute, waterStats.wateringSession[ii].mlUsed);
    }
  }

}

void waterStatsLogger::clearStats()
{
  for (int i = startAddr; i < startAddr + sizeof(waterStatsChanStorage_); ++i) {
    EEPROM.write(i, 0x00);
  }
}
	