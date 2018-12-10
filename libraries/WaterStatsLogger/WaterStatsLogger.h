#ifndef __WATER_STATS_LOGGER_H__
#define __WATER_STATS_LOGGER_H__

#ifndef STAT_LOG_SIZE
#define STAT_LOG_SIZE  10
#endif

#ifndef STAT_MSG_LEN
#define STAT_MSG_LEN 21
#endif

#include <Time.h>

// LOG: Statistiques de consommation d'eau
// fichiers placés
typedef struct
{
  time_t dateTime; // 4 bytes date representation
  uint16_t mlUsed; // 2 bytes
   
} wateringSession_;

typedef struct 
{
  unsigned long totalMililitres;
  uint16_t  nbWaterings;

  wateringSession_ wateringSession[STAT_LOG_SIZE];
  uint8_t watSessionLogLine; // identifiant de ligne courant dans le tableau de log
  
} waterStatsChanStorage_;


class waterStatsLogger
{
  protected:
  
  int startAddr;
  
  waterStatsChanStorage_ waterStats;
  
  public:

  waterStatsLogger(int startAddr);
  
  virtual ~waterStatsLogger(){}
  
  /**
   * enregistremnt des stats dans l'eeprom
   */
  void saveSessionStats(uint16_t totalMililitres);

  void readLogStats(char** text, uint8_t& len, unsigned long& totalMililitres, uint16_t&  nbWaterings);

  void clearStats();
};

#endif //__WATER_STATS_LOGGER_H__
