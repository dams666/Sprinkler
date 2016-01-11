#ifndef __VALVES_H__
#define __VALVES_H__

#include <arduino.h> 

class MOD_valves_
{
  public:

  int *state;
    
  protected:

  int mainPin;
  int *pins;
  int fertilizerPin;
  int maxOpened;

  public:
  
  MOD_valves_();
  
  int  getNbValvesOpened();
    
  void changeValveState();
  
  void purgeTransitionalCircuit();
    
  void openMainValve();
    
  void closeMainValve();
   
  void closeAllValves();

};


#endif
