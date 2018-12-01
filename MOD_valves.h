#ifndef __VALVES_H__
#define __VALVES_H__

#include <arduino.h> 
#include "module.h"
#include "config.h"

class MOD_valves_ : public Module
{
  public:

  int state[MAX_CHANNELS_];
    
  protected:

  int pins[MAX_CHANNELS_];
  //int fertilizerPin;

  public:
  
  MOD_valves_();

  void reset();
  
  int  getNbValvesOpened();
    
  bool changeValveState();
    
  bool openMainValve();
    
  void closeMainValve();
   
  void closeAllValves();

};


#endif
