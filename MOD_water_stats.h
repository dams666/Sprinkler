#ifndef __WATER_STATS_H__
#define __WATER_STATS_H__

#define MAX_MILILITRES_PER_VALVE  500

#include <arduino.h>

enum  waterFlow {
  WATER_FLOWING,
  WATER_STOPPED,
  WATER_BLOCKED,
  WATER_OVERFLOW
} ;

class MOD_waterStats_
{
  protected :
  
  unsigned long flowStatsOldTime;

  unsigned long *totalMililitresSession;
  unsigned long *lastTotalMililitresSession;
  
  unsigned long *totalMililitres;
  unsigned long *nbWaterings;
  
  int waterFlow;
  
  byte flowSensorInterrupt;
  byte flowSensorPin;
  
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

  void show();

  void printFlow();

  /* Calcul des statistiques de consommation d'eau une fois une vanne ouverte
   ATTENTION : on part du principe qu'une seule vanne est ouverte à la fois.
  */ 
  int calcFlow();

};


#endif

