#ifndef __VALVES_H__
#define __VALVES_H__

#include <arduino.h> 
#include "module.h"
#include "config.h"


/*
 * Module chapeau. Il déclence les autres modules 
 */
class MOD_valves_ : public Module
{
  public:

  bool state[MAX_CHANNELS_];
  volatile bool stateMain;
  
  protected:

  uint8_t pins[MAX_CHANNELS_];

  public:
  
  MOD_valves_();

  bool reset();
  bool start();
  
  uint8_t  getNbValvesOpened();

  /**
   * Déclenchement du module water stats
   */
  bool openMainValve();
    
  bool closeMainValve();
   
};


#endif
