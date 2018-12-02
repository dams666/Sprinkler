#include "main.h"
#include "MOD_valves.h"
#include "MOD_moisture_sensors.h"
#include "MOD_water_stats.h"

  MOD_valves_::MOD_valves_()
  { 
    memset (pins, 0, sizeof(short) * MAX_CHANNELS_);
    
    if (MAX_CHANNELS_>0) pins[0] = VALVE_1_PIN;    
    if (MAX_CHANNELS_>1) pins[1] = VALVE_2_PIN;
    if (MAX_CHANNELS_>2) pins[2] = VALVE_3_PIN;
    if (MAX_CHANNELS_>3) pins[3] = VALVE_4_PIN;
    if (MAX_CHANNELS_>4) pins[4] = VALVE_5_PIN;
    if (MAX_CHANNELS_>5) pins[5] = VALVE_6_PIN;

    //fertilizerPin = 39;
   
    pinMode(VALVE_M_PIN, OUTPUT);
    //pinMode(fertilizerPin, OUTPUT);
    
    for (int thisPin = 0; thisPin < MAX_CHANNELS_; thisPin++)
    {
      pinMode(pins[thisPin], OUTPUT);
    }

    reset();
  }

  void MOD_valves_::reset()
  {
    for (int thisPin = 0; thisPin < MAX_CHANNELS_; thisPin++)
    {
      state[thisPin] = 0;
      digitalWrite(pins[thisPin], VALVE_OFF);
    }
    
    digitalWrite(VALVE_M_PIN, VALVE_OFF);
    //digitalWrite(fertilizerPin, VALVE_OFF);
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
    if (!__channelStorage[__curChannel].active)
      return false;
             
    if (__MOD_moistureSensors->state[__curChannel]) // activation du moisture sensor
    {
      if (getNbValvesOpened() < MAX_VALVES_OPENED && !state[__curChannel] )
      {
        DEBUG_PRINT(F(" => OPEN VALVE "));
        DEBUG_PRINTLN(__curChannel);

        String s;
        s = F("VALVE ");
        s+= (1 + __curChannel);
        s+= F(" OPENED");

        LCD_CLEAR();
        LCD_PRINT(0,0,s);
    
        digitalWrite(pins[__curChannel], VALVE_ON); // activation de la valve
        state[__curChannel] = 1;

        delay(200);
        openMainValve();

      }
    }
    else {
      // fermeture du moisture sensor

      if (state[__curChannel]) 
      {  
        DEBUG_PRINT(F(" => CLOSE VALVE "));
        DEBUG_PRINTLN(1 + __curChannel);
        
        String s;
        s = F("VALVE ");
        s+= (1 + __curChannel);
        s+= F(" CLOSED");

        LCD_PRINT(0,0,s);
              
        if (getNbValvesOpened() == 1)
          closeMainValve();
        
        // fermeture de la vanne         
        digitalWrite(pins[__curChannel], VALVE_OFF);

        state[__curChannel] = 0;
        __gui->centerText(F("STATISTICS"));
        __MOD_waterStats->show(1);
         delay(1000);
      } 
    }
    return true;
  }  
  bool MOD_valves_::openMainValve()
  {
     if (!__channelStorage[__curChannel].active)
      return false;
      
     __MOD_waterStats->start();

     digitalWrite(VALVE_M_PIN, VALVE_ON); // activation de la valve principale

     //digitalWrite(fertilizerPin, VALVE_ON); // activation de la valve principale

     return true;
  }
  
  void MOD_valves_::closeMainValve()
  {
    setState(PRGM_STATE_CLOSING_MAIN_VALVE, 0);
    
    DEBUG_PRINTLN(F(" => CLOSE MAIN VALVE "));
    digitalWrite(VALVE_M_PIN, VALVE_OFF); // fermeture de la vanne principale

    // on laisse le temps a l'eau de s'�couler, et de faire baisser la pression dans les tuyaux
    while(__MOD_waterStats->calcFlow() == WATER_FLOWING)
      delay(500);
    
    DEBUG_PRINTLN(F(" WATER STOPPED"));  

    //digitalWrite(fertilizerPin, VALVE_OFF); // activation de la valve principale
  }
 
  void MOD_valves_::closeAllValves()
  {
    closeMainValve();

    for (int thisPin = 0; thisPin < MAX_CHANNELS_; thisPin++)
    {
      digitalWrite(pins[thisPin], VALVE_OFF); // fermeture de la valve
    }    
  }

