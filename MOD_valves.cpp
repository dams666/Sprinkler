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

  bool MOD_valves_::reset()
  {
    if (!closeMainValve())
      return false;

    for (int thisPin = 0; thisPin < MAX_CHANNELS_; thisPin++)
    {
      state[thisPin] = 0;
      digitalWrite(pins[thisPin], VALVE_OFF); // fermeture de la valve
    }
    return true;
  }

  int  MOD_valves_::getNbValvesOpened()
  {
    int res = 0;
    for (int thisPin = 0; thisPin < MAX_CHANNELS_; thisPin++)
      res +=state[thisPin];

    return res;
  }
  
  bool MOD_valves_::start()
  {    
    if (!__channelConf[__curChannel].active)
      return false;
             
    if (__MOD_moistureSensors->state[__curChannel]) // activation du moisture sensor
    {
      if (getNbValvesOpened() < MAX_VALVES_OPENED && !state[__curChannel] )
      {
        DEBUG_PRINT(F(" => OPEN VALVE "));
        DEBUG_PRINTLN(__curChannel);

        char str[LCD_COLUMNS_ + 1];
        sprintf_P(str, PSTR("VALVE %d OPENED"), 1 + __curChannel);

        LCD_CLEAR();
        LCD_PRINT(0,0,str);
    
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
        
        char str[LCD_COLUMNS_ + 1];
        sprintf_P(str, PSTR("VALVE %d CLOSED"), 1 + __curChannel);

        LCD_PRINT(0,0,str);
              
        if (getNbValvesOpened() == 1)
          closeMainValve();
        
        // fermeture de la vanne secondaire
        digitalWrite(pins[__curChannel], VALVE_OFF);
        state[__curChannel] = 0;
        __MOD_waterStats->saveSessionStats();
        __gui->centerText(F("STATISTICS"));
        __MOD_waterStats->show(1);
        
         delay(1000);
      } 
    }
    return true;
  }  
  bool MOD_valves_::openMainValve()
  {
     if (!__channelConf[__curChannel].active)
      return false;
      
     __MOD_waterStats->start();

     digitalWrite(VALVE_M_PIN, VALVE_ON); // activation de la valve principale
      stateMain = 1;

     return true;
  }
  
  bool MOD_valves_::closeMainValve()
  {
    
    DEBUG_PRINTLN(F(" => CLOSING MAIN VALVE "));
    digitalWrite(VALVE_M_PIN, VALVE_OFF); // fermeture de la vanne principale

    int attempts = 0;
    // on laisse le temps a l'eau de s'écouler, et de faire baisser la pression dans les tuyaux
    while(__MOD_waterStats->calcFlow() == WATER_FLOWING) {
      attempts++;
      if (attempts > 20)
        return false;
      delay(500);
    }
    stateMain = 0
    DEBUG_PRINTLN(F(" WATER STOPPED"));   
    return true;  
  }
 

