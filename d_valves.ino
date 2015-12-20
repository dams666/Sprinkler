
  void changeValveState()
  {       
    if (moistureSensorState[numValveToInspect]) // activation du moisture sensor
    {
      if (getNbValvesOpened() < maxValvesOpened && !valveState[numValveToInspect] )
      {
        programState = INSPECTING_FOR_CHANGES;

        msg = " => OPEN VALVE ";
        msg+= numValveToInspect;

        digitalWrite(valvePins[numValveToInspect], RELAY_ON); // activation de la valve
        valveState[numValveToInspect] = 1;
        // initialisation des statistiques de consommation d'eau;
        flowStatsOldTime[numValveToInspect] = millis();
        
#ifdef WITH_SERIAL  
        Serial.println(msg);
#endif

        delay(200);

        digitalWrite(mainValvePin, LOW); // activation de la valve principale

      }
    } 
    else { // fermeture du moisture sensor

      if (valveState[numValveToInspect]) 
      {
        msg = " => CLOSE VALVE ";
        msg+= numValveToInspect;

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
        //lastTotalMilliLitres[numValveToInspect] = totalMilliLitres[numValveToInspect];
                
#ifdef WITH_SERIAL  
        Serial.println(msg);
#endif

        calcFlowStats();

        delay(200);

      } 
    }   

  }


  int  getNbValvesOpened()
  {
    int res = 0;
    for (int thisPin = 0; thisPin < valveCount; thisPin++)
      res +=valveState[thisPin];

    return res;
  }
