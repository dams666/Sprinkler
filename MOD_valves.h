#ifndef __VALVES_H__
#define __VALVES_H__

#include <arduino.h> 
#include <module.h>
#include <ModuleConfig.h>

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

  bool stop();
  bool start() {return true;}
  bool execute();

  /**
   * Détection de reprise suite à l'extinction d'une autre vanne
   */
  bool hasStateChanged();
  
  uint8_t  getNbValvesOpened();

  /**
   * Déclenchement du module water stats
   */
  bool openMainValve();
    
  bool closeMainValve();
   
};


#endif
