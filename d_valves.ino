  void initValves()
  {
    mainValvePin = 53;

    valvePins[0] = 51;
    
    if (NB_VALVES > 1) valvePins[1] = 49;
    if (NB_VALVES > 2) valvePins[2] = 47;
    if (NB_VALVES > 3) valvePins[3] = 45;
    if (NB_VALVES > 4) valvePins[4] = 43;
    if (NB_VALVES > 5) valvePins[5] = 41;

    fertilizerValvePin = 39;

    pinMode(mainValvePin, OUTPUT);
    pinMode(fertilizerValvePin, OUTPUT);

    for (int thisPin = 0; thisPin < NB_VALVES; thisPin++)
    {
      pinMode(valvePins[thisPin], OUTPUT);

      digitalWrite(valvePins[thisPin], RELAY_OFF);
      valveState[thisPin]                = 0;
    }
    digitalWrite(mainValvePin, HIGH);
    
    numValveToInspect   = 0;
    maxValvesOpened     = 1;
  }
  
  void changeValveState()
  {       
    if (moistureSensorState[numValveToInspect]) // activation du moisture sensor
    {
      if (getNbValvesOpened() < maxValvesOpened && !valveState[numValveToInspect] )
      {
        programState = INSPECTING_FOR_CHANGES;

        DEBUG_PRINT(" => OPEN VALVE ");
        DEBUG_PRINTLN(numValveToInspect);

        String s;
        s = "VALVE ";
        s+= numValveToInspect;
        s+= " OPENED";
        
        lcd.clear();
        lcd.setCursor(0,0); //Start at character 4 on line 0
        lcd.print(s);
    
        digitalWrite(valvePins[numValveToInspect], RELAY_ON); // activation de la valve
        valveState[numValveToInspect] = 1;

        resetWaterStats();
        
        delay(200);

        openMainValve();

      }
    } 
    else { // fermeture du moisture sensor

      if (valveState[numValveToInspect]) 
      {  
        DEBUG_PRINT(" => CLOSE VALVE ");
        DEBUG_PRINTLN(numValveToInspect);
        
        String s;
        s = "VALVE ";
        s+= numValveToInspect;
        s+= " CLOSED";
        lcd.clear(); 
        lcd.setCursor(0,0); //Start at character 4 on line 0
        lcd.print(s);
              
        if (getNbValvesOpened() == 1)
        {
          closeMainValve();
          delay(100);
        }
        
        // fermeture de la vanne         
        digitalWrite(valvePins[numValveToInspect], RELAY_OFF);

        valveState[numValveToInspect] = 0;

        showWaterStats();

         delay(1000);
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

  void purgeTransitionalCircuit()
  {
    digitalWrite(valvePins[0], RELAY_ON); // activation de la valve
    delay(500);

    digitalWrite(valvePins[0], RELAY_OFF); // fermeture de la valve   
    delay(500);
  }
  
  void openMainValve()
  {
     // initialisation des statistiques de consommation d'eau;
     flowStatsOldTime = millis();
        
     digitalWrite(mainValvePin, RELAY_ON); // activation de la valve principale

     digitalWrite(fertilizerValvePin, RELAY_ON); // activation de la valve principale

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

    digitalWrite(fertilizerValvePin, RELAY_OFF); // activation de la valve principale
  }
 
  void closeAllValves()
  {
    closeMainValve();

    for (int thisPin = 0; thisPin < NB_VALVES; thisPin++)
    {
      digitalWrite(valvePins[thisPin], RELAY_OFF); // fermeture de la valve
    }    
    digitalWrite(fertilizerValvePin, RELAY_OFF); // activation de la valve principale
  }

