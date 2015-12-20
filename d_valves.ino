
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
        flowStatsOldTime[numValveToInspect] = millis();

        delay(200);

        digitalWrite(mainValvePin, LOW); // activation de la valve principale

      }
    } 
    else { // fermeture du moisture sensor

      if (valveState[numValveToInspect]) 
      {
        
        DEBUG_PRINT(" => CLOSE VALVE ");
        DEBUG_PRINTLN(numValveToInspect);

        if (getNbValvesOpened() == 1)
        {
          programState = CLOSING_MAIN_VALVE;

          digitalWrite(mainValvePin, HIGH); // fermeture de la vanne principale
          delay(2000); // on lasse le temps a l'eau de s'écouler, et de faire baisser la pression dans les tuyaux
        }

        // fermeture de la vanne         
        digitalWrite(valvePins[numValveToInspect], RELAY_OFF);

        valveState[numValveToInspect] = 0;
        
        flowStatsOldTime[numValveToInspect] = millis();

        calcFlowStats();

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
