#ifndef __WATER_STATS_H__
#define __WATER_STATS_H__

#include <Arduino.h>
#include "module.h"
#include "config.h"

#include <Time.h>

enum  waterFlow_ {
  WATER_FLOWING,
  WATER_STOPPED,
  WATER_BLOCKED,
  WATER_OVERFLOW
};


// LOG: Statistiques de consommation d'eau
// fichiers placés
typedef struct
{
  time_t dateTime; // 4 bytes date representation
  unsigned int mlUsed; // 2 bytes
   
} wateringSession_;

typedef struct 
{
  unsigned long totalMililitres;
  unsigned int  nbWaterings;

  wateringSession_ wateringSession[STAT_LOG_SIZE];
  unsigned short watSessionLogLine; // identifiant de ligne courant dans le tableau de log
  
} waterStatsChanStorage_;

void readCurChannelStats(waterStatsChanStorage_* waterStats);

class MOD_waterStats_ : public Module
{
  protected :
  
  unsigned long flowStatsOldTime;

  /**
   * Une session est délimitée à deux passages consécutifs sur le même canal de vanne.
   * Une session est entrecoupée d'une phase de sommeil
   */
  unsigned int totalMililitresSession[MAX_CHANNELS_];
  unsigned int lastTotalMililitresSession[MAX_CHANNELS_];

  /**
   * Flux courant en L/min
   */
  float flowRate[MAX_CHANNELS_];

  waterFlow_ waterFlow;
  
  float flowSensorCalibrationFactor;
  volatile byte flowPulseCount;  
    
  int incoherentPulseCount;
  unsigned long lastIncoherentPulseCountTime;

  public: 
    
/*
  Flow sensor Insterrupt Service Routine
*/
  void flowIncPulseCounter();

  MOD_waterStats_();

  bool reset();

  /** 
   *  Démarrage de la session de statistiques
   */
  bool start();
  
  void show(int row);
  void printFlow();
  
  /**
   * enregistremnt des stats dans l'eeprom
   */
  void saveSessionStats();
  
  /* Calcul des statistiques de consommation d'eau une fois une vanne ouverte
   ATTENTION : on part du principe qu'une seule vanne est ouverte à la fois.
  */ 
  int calcFlow();

};


#endif

