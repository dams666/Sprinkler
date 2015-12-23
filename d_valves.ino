  
  void changeValveState()
  {       
    if (moistureSensorState[numValveToInspect]) // activation du moisture sensor
    {
      if (getNbValvesOpened() < maxValvesOpened && !valveState[numValveToInspect] )
      {
        programState = INSPECTING_FOR_CHANGES;

        DEBUG_PRINT(" => OPEN VALVE ");
        DEBUG_PRINTLN(numValveToInspect);
        
        digitalWrite(valvePins[numValveToInspect], RELAY_ON); // activation de la valve
        valveState[numValveToInspect] = 1;
        // initialisation des statistiques de consommation d'eau;
        flowStatsOldTime = millis();
        nbWaterings[numValveToInspect]++;
        
        delay(200);

        digitalWrite(mainValvePin, LOW); // activation de la valve principale

      }
    } 
    else { // fermeture du moisture sensor

      if (valveState[numValveToInspect]) 
      {
        
        if (getNbValvesOpened() == 1)
          closeMainValve();
 
        DEBUG_PRINT(" => CLOSE VALVE ");
        DEBUG_PRINTLN(numValveToInspect);
        
        // fermeture de la vanne         
        digitalWrite(valvePins[numValveToInspect], RELAY_OFF);

        valveState[numValveToInspect] = 0;

         delay(200);
      } 
    }   

  }


  int  getNbValvesOpened()
  {
    int res = 0;
    for (int thisPin = 0; thisPin < NB_VALVES; thisPin++)
      res +=valveState[thisPin];

    return res;
  }

  void closeMainValve()
  {
    programState = CLOSING_MAIN_VALVE;

    DEBUG_PRINTLN(" => CLOSE MAIN VALVE ");
    digitalWrite(mainValvePin, RELAY_OFF); // fermeture de la vanne principale

    // on laisse le temps a l'eau de s'écouler, et de faire baisser la pression dans les tuyaux
    while(calcFlowStats() != WATER_STOPPED)
      delay(500);
    DEBUG_PRINTLN(" WATER STOPPED");  
  }
 

