#ifndef __WATER_STATS_H__
#define __WATER_STATS_H__

#include <Arduino.h>
#include "module.h"
#include "config.h"

enum  waterFlow {
  WATER_FLOWING,
  WATER_STOPPED,
  WATER_BLOCKED,
  WATER_OVERFLOW
};



class MOD_waterStats_ : public Module
{
  protected :
  
  unsigned long flowStatsOldTime;

  unsigned int totalMililitresSession[MAX_CHANNELS_];
  unsigned int lastTotalMililitresSession[MAX_CHANNELS_];

  float flowRate[MAX_CHANNELS_]; // in L/min
  
  int waterFlow;
  
  byte flowSensorInterrupt;
  
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

  void reset();

  void start();
  void show(int);

  void printFlow();

  /* Calcul des statistiques de consommation d'eau une fois une vanne ouverte
   ATTENTION : on part du principe qu'une seule vanne est ouverte à la fois.
  */ 
  int calcFlow();

};


#endif

