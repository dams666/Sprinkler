#include "main.h"
#include "MOD_valves.h"
#include "MOD_moisture_sensors.h"
#include "MOD_water_stats.h"

  MOD_valves_::MOD_valves_()
  { 
    state = new int[__nbChannels];
    pins = new int[__nbChannels];

    memset (state, 0, sizeof(int) * __nbChannels);
    memset (pins, 0, sizeof(int) * __nbChannels);
    
    mainPin = 53;

    pins[0] = 51;    
    if (__nbChannels > 1) pins[1] = 49;
    if (__nbChannels > 2) pins[2] = 47;
    if (__nbChannels > 3) pins[3] = 45;
    if (__nbChannels > 4) pins[4] = 43;
    if (__nbChannels > 5) pins[5] = 41;

    fertilizerPin = 39;
   
    pinMode(mainPin, OUTPUT);
    pinMode(fertilizerPin, OUTPUT);
    
    for (int thisPin = 0; thisPin < __nbChannels; thisPin++)
    {
      pinMode(pins[thisPin], OUTPUT);
      digitalWrite(pins[thisPin], RELAY_OFF);
    }
    digitalWrite(mainPin, RELAY_OFF);
    digitalWrite(fertilizerPin, RELAY_OFF);
    
    maxOpened      = 1;
  }


  int  MOD_valves_::getNbValvesOpened()
  {
    int res = 0;
    for (int thisPin = 0; thisPin < __nbChannels; thisPin++)
      res +=state[thisPin];

    return res;
  }
  
  void MOD_valves_::changeValveState()
  {       
    if (__MOD_moistureSensors->state[__curChannel]) // activation du moisture sensor
    {
      if (getNbValvesOpened() < maxOpened && !state[__curChannel] )
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
        {
          closeMainValve();
          delay(100);
        }
        
        // fermeture de la vanne         
        digitalWrite(pins[__curChannel], RELAY_OFF);

        state[__curChannel] = 0;

        __MOD_waterStats->show();

         delay(1000);
      } 
    }   

  }

  void MOD_valves_::purgeTransitionalCircuit()
  {
    LCD_PRINT(0,3, "PURGING WATER...");
        
    digitalWrite(pins[0], RELAY_ON); // activation de la valve
    delay(1000);

    digitalWrite(pins[0], RELAY_OFF); // fermeture de la valve   
    delay(1000);
  }
  
  void MOD_valves_::openMainValve()
  {
     __MOD_waterStats->reset();
        
     digitalWrite(mainPin, RELAY_ON); // activation de la valve principale

     digitalWrite(fertilizerPin, RELAY_ON); // activation de la valve principale

  }
  
  void MOD_valves_::closeMainValve()
  {
    __programState = PRGM_STATE_CLOSING_MAIN_VALVE;

    DEBUG_PRINTLN(" => CLOSE MAIN VALVE ");
    digitalWrite(mainPin, RELAY_OFF); // fermeture de la vanne principale

    // on laisse le temps a l'eau de s'�couler, et de faire baisser la pression dans les tuyaux
    while(__MOD_waterStats->calcFlow() != WATER_STOPPED)
    {
      delay(500);
    }
    
    DEBUG_PRINTLN(" WATER STOPPED");  

    digitalWrite(fertilizerPin, RELAY_OFF); // activation de la valve principale
  }
 
  void MOD_valves_::closeAllValves()
  {
    closeMainValve();

    for (int thisPin = 0; thisPin < __nbChannels; thisPin++)
    {
      digitalWrite(pins[thisPin], RELAY_OFF); // fermeture de la valve
    }    
  }

