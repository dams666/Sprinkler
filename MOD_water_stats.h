#ifndef __WATER_STATS_H__
#define __WATER_STATS_H__

#include <Arduino.h>
#include "module.h"
#include "config.h"

enum  waterFlow_ {
  WATER_FLOWING,
  WATER_STOPPED,
  WATER_BLOCKED,
  WATER_OVERFLOW
};

class MOD_waterStats_ : public Module
{
  protected :
  
  unsigned long flowStatsOldTime;

  /**
   * Une session est délimitée à deux passages consécutifs sur le même canal de vanne.
   * Une session est entrecoupée d'une phase de sommeil
   */
  uint16_t totalMililitresSession[MAX_CHANNELS_];
  uint16_t lastTotalMililitresSession[MAX_CHANNELS_];

  /**
   * Flux courant en L/min
   */
  float flowRate[MAX_CHANNELS_];

  waterFlow_ waterFlow;
  
  float flowSensorCalibrationFactor;
  volatile uint8_t flowPulseCount;  
    
  uint8_t incoherentPulseCount;
  unsigned long lastIncoherentPulseCountTime;

  public: 
    
  MOD_waterStats_();

  /** 
   *  Démarrage de la session de statistiques
   */
  bool start();
  
  bool stop();
  
  /*
  Flow sensor Insterrupt Service Routine
  */
  bool execute();

  
  void show(char*);
  void printFlow();

  void saveSessionStats();
  
  /* Calcul des statistiques de consommation d'eau une fois une vanne ouverte
   ATTENTION : on part du principe qu'une seule vanne est ouverte à la fois.
  */ 
  waterFlow_ calcFlow();

};


#endif

