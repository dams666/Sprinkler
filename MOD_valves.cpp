#include "main.h"
#include "MOD_valves.h"
#include "MOD_moisture_sensors.h"
#include "MOD_water_stats.h"

  MOD_valves_::MOD_valves_()
  { 
    state = new int[MAX_CHANNELS_];
    pins = new int[MAX_CHANNELS_];

    memset (state, 0, sizeof(int) * MAX_CHANNELS_);
    memset (pins, 0, sizeof(int) * MAX_CHANNELS_);
    
    mainPin = 53;

    pins[0] = 51;    
    pins[1] = 49;
    pins[2] = 47;
    pins[3] = 45;
    pins[4] = 43;
    pins[5] = 41;

    fertilizerPin = 39;
   
    pinMode(mainPin, OUTPUT);
    pinMode(fertilizerPin, OUTPUT);
    
    for (int thisPin = 0; thisPin < MAX_CHANNELS_; thisPin++)
    {
      pinMode(pins[thisPin], OUTPUT);
      digitalWrite(pins[thisPin], RELAY_OFF);
    }
    digitalWrite(mainPin, RELAY_OFF);
    digitalWrite(fertilizerPin, RELAY_OFF);

  }


  int  MOD_valves_::getNbValvesOpened()
  {
    int res = 0;
    for (int thisPin = 0; thisPin < MAX_CHANNELS_; thisPin++)
      res +=state[thisPin];

    return res;
  }
  
  bool MOD_valves_::changeValveState()
  {
    if (!__channelActivated[__curChannel])
      return false;
             
    if (__MOD_moistureSensors->state[__curChannel]) // activation du moisture sensor
    {
      if (getNbValvesOpened() < MAX_VALVES_OPENED && !state[__curChannel] )
      {
        __programState = PRGM_STATE_INSPECTING_FOR_CHANGES;

        DEBUG_PRINT(" => OPEN VALVE ");
        DEBUG_PRINTLN(__curChannel);

        String s;
        s = "VALVE ";
        s+= (1 + __curChannel);
        s+= " OPENED";

        __LCD->clear();
        LCD_PRINT(0,0,s);
    
        digitalWrite(pins[__curChannel], RELAY_ON); // activation de la valve
        state[__curChannel] = 1;

        delay(200);
        openMainValve();

      }
    } 
    else { // fermeture du moisture sensor

      if (state[__curChannel]) 
      {  
        DEBUG_PRINT(" => CLOSE VALVE ");
        DEBUG_PRINTLN(1 + __curChannel);
        
        String s;
        s = "VALVE ";
        s+= (1 + __curChannel);
        s+= " CLOSED";

        LCD_PRINT(0,0,s);
              
        if (getNbValvesOpened() == 1)
          closeMainValve();
        
        // fermeture de la vanne         
        digitalWrite(pins[__curChannel], RELAY_OFF);

        state[__curChannel] = 0;

        __MOD_waterStats->show();

         delay(1000);
      } 
    }   
    return true;
  }

  void MOD_valves_::purgeTransitionalCircuit()
  {
    LCD_PRINT(0,3, "PURGING WATER...");
        
    digitalWrite(pins[0], RELAY_ON); // activation de la valve
    delay(1000);

    digitalWrite(pins[0], RELAY_OFF); // fermeture de la valve   
    delay(1000);
  }
  
  bool MOD_valves_::openMainValve()
  {
     if (!__channelActivated[__curChannel])
      return false;
      
     __MOD_waterStats->reset();

     digitalWrite(mainPin, RELAY_ON); // activation de la valve principale

     digitalWrite(fertilizerPin, RELAY_ON); // activation de la valve principale

     return true;
  }
  
  void MOD_valves_::closeMainValve()
  {
    __programState = PRGM_STATE_CLOSING_MAIN_VALVE;

    DEBUG_PRINTLN(" => CLOSE MAIN VALVE ");
    digitalWrite(mainPin, RELAY_OFF); // fermeture de la vanne principale

    // on laisse le temps a l'eau de s'�couler, et de faire baisser la pression dans les tuyaux
    while(__MOD_waterStats->calcFlow() == WATER_FLOWING)
      delay(500);
    
    DEBUG_PRINTLN(" WATER STOPPED");  

    digitalWrite(fertilizerPin, RELAY_OFF); // activation de la valve principale
  }
 
  void MOD_valves_::closeAllValves()
  {
    closeMainValve();

    for (int thisPin = 0; thisPin < MAX_CHANNELS_; thisPin++)
    {
      digitalWrite(pins[thisPin], RELAY_OFF); // fermeture de la valve
    }    
  }

