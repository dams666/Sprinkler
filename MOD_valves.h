#ifndef __VALVES_H__
#define __VALVES_H__

#include <arduino.h> 

#define MAX_VALVES_OPENED 1

class MOD_valves_
{
  public:

  int *state;
    
  protected:

  int mainPin;
  int *pins;
  int fertilizerPin;

  public:
  
  MOD_valves_();
  
  int  getNbValvesOpened();
    
  bool changeValveState();
  
  void purgeTransitionalCircuit();
    
  bool openMainValve();
    
  void closeMainValve();
   
  void closeAllValves();

};


#endif
