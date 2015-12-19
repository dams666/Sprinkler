
#define WITH_SERIAL

#define NB_VALVES 6


byte statusLed    = 13;

class sprinkler
{
protected:

  //------------------------------------------------------------------------------------
  // PINS
  //------------------------------------------------------------------------------------

  int mainValvePin;

  int valvePins[NB_VALVES];

  int moistureSensorPins[NB_VALVES];

  int moistureSensorActivationPin;

  byte flowSensorInterrupt;
  byte flowSensorPin;

  //------------------------------------------------------------------------------------
  // VALVES VARIABLES
  //------------------------------------------------------------------------------------

  int valveCount;

  int valveState[NB_VALVES];

  int numValveToInspect; // identifiant courant de valve à inspecter ( pour l'état INSPECTING_FOR_CHANGES )

  int maxValvesOpened;

  //------------------------------------------------------------------------------------
  // MOISTURE SENSORS VARIABLES
  //------------------------------------------------------------------------------------

  int moistureSensorState[NB_VALVES];        // current moistureSensorState of the machine
  int prevMoistureSensorState[NB_VALVES];         // previous moistureSensorState of the machine

  //------------------------------------------------------------------------------------
  // PROGRAM STATE
  //------------------------------------------------------------------------------------

  enum  programStates {
    INITIALIZING,
    ACTIVATING_MOISTURE_SENSORS,
    READING_MOISTURE_SENSORS,
    INSPECTING_FOR_CHANGES,
    CLOSING_MAIN_VALVE,
    ALERT
  } 
  programState;

  /*
  On définit un état spécifique des lors qu'il dure un certain temps
   
   INITIALIZING : 
   
   après allumage ou reboot de l'arduino après plantage. Les vannes secondaires sont ouvertes puis refermées pour dissiper l'eau 
   qui se serait éventuellement accumulée dans les tuyaux entre la vanne princuppale et la vanne secondaire
   
   ETAT(S) PRECEDENT(S) :
   AUCUN
   ETAT(S) SUIVANT(S)   : 
   ACTIVATING_MOISTURE_SENSORS 
   
   ACTIVATING_MOISTURE_SENSORS :
   
   activation de l'ensemble des détecteurs d'humidité. On laisse un court instant passer avant de lire les résultats (cf état READING_MOISTURE_SENSORS)
   
   ETAT(S) PRECEDENT(S) :
   INITIALIZING
   INSPECTING FOR CHANGES : si toutes les vannes secondaires ont été fermées, on passe en veille
   
   ETAT(S) SUIVANT(S)   : 
   READING_MOISTURE_SENSORS
   
   READING_MOISTURE_SENSORS :
   
   lecture des détecteurs d'humidité
   
   ETAT(S) PRECEDENT(S) : 
   ACTIVATING_MOISTURE_SENSORS
   INSPECTING FOR CHANGES : si une vanne secondaire est ouverte, donc une plante est arrosée
   
   ETAT(S) SUIVANT(S)   : 
   INSPECTING_FOR_CHANGES
   
   INSPECTING_FOR_CHANGES :
   
   inspection des changements de valeur des détecteurs d'humidité, et ouverture / fermeture des vannes
   
   ETAT(S) PRECEDENT(S) : 
   INSPECTING FOR CHANGES : si incrémentation de l'identifiant de vanne secondaire à inspecter
   
   ETAT(S) SUIVANT(S)   : 
   INSPECTING FOR CHANGES : si incrémentation de l'identifiant de vanne secondaire à inspecter
   CLOSING_MAIN_VALVE : si une vanne secondaire doit être fermée, on ferme d'abord la vanne principale et on attend que l'eau ne circule plus
   
   CLOSING_MAIN_VALVE :
   
   avant de fermer une vanne secondaire, on ferme la vanne principale et on attend un instant que l'eau ne circule plus (l'effet de fermeture n'est pas toujours immédiat).
   Comme la fermeture n'est pas immédiate, cela permet de maintenir une pression nulle dans les tuyaux entre la vanne principale et les vannes intermédiaires.
   
   ETAT(S) PRECEDENT(S) : 
   INSPECTING FOR CHANGES
   
   ETAT(S) SUIVANT(S)   : 
   
   */


  //------------------------------------------------------------------------------------
  // WATER CONSUMTION STATS
  //------------------------------------------------------------------------------------

  float flowSensorCalibrationFactor;
  volatile byte flowPulseCount[NB_VALVES];  

  unsigned long totalMilliLitres[NB_VALVES];
  unsigned long lastTotalMilliLitres[NB_VALVES];
  
  unsigned long flowStatsOldTime[NB_VALVES];
  
  int incoherentPulseCount;
  unsigned long lastIncoherentPulseCountTime;
  
  //------------------------------------------------------------------------------------
  // OTHER
  //------------------------------------------------------------------------------------

  String msg;

public:

  sprinkler()
  {
    //------------------------------------------------------------------------------------
    // DEFINE PINS
    //------------------------------------------------------------------------------------

    mainValvePin = 53;
    moistureSensorActivationPin = 32;  

    valvePins[0] = 51;
    valvePins[1] = 49;
    valvePins[2] = 47;
    valvePins[3] = 45;
    valvePins[4] = 43;
    valvePins[5] = 41;

    moistureSensorPins[0] = 52;
    moistureSensorPins[1] = 50;
    moistureSensorPins[2] = 48;
    moistureSensorPins[3] = 46;
    moistureSensorPins[4] = 44;
    moistureSensorPins[5] = 42;

    flowSensorInterrupt = 0;  // 0 = digital pin 2
    flowSensorPin       = 2; 

    //------------------------------------------------------------------------------------ 
    // DEFINE PROGRAM STATE
    //------------------------------------------------------------------------------------

    programState = INITIALIZING;

    for (int ii = 0; ii< NB_VALVES; ++ii)
    {
      valveState[ii]                = 0;
      moistureSensorState[ii]       = 0;
      prevMoistureSensorState[ii]   = 0;
    }

    //------------------------------------------------------------------------------------
    // DEFINE VALVES VARIABLES
    //------------------------------------------------------------------------------------

    valveCount          = 5; 
    numValveToInspect   = 0;
    maxValvesOpened     = 1;

    //------------------------------------------------------------------------------------
    // DEFINE FLOW SENSOR VARIABLES
    //------------------------------------------------------------------------------------

    // The hall-effect flow sensor outputs approximately 4.5 pulses per second per
    // litre/minute of flow.
    flowSensorCalibrationFactor = 4.5f;

    incoherentPulseCount = 0;
    lastIncoherentPulseCountTime = 0;
    
    for (int ii = 0; ii< NB_VALVES; ++ii)
    {
      totalMilliLitres[ii]       = 0;
      lastTotalMilliLitres[ii]   = 0;
      flowStatsOldTime[ii]       = 0;
      flowPulseCount[ii]         = 0;
    }


    //------------------------------------------------------------------------------------
    // INIT PINS
    //------------------------------------------------------------------------------------

    pinMode(mainValvePin, OUTPUT);
    pinMode(moistureSensorActivationPin, OUTPUT);

    // the array elements are numbered from 0 to (pinCount - 1).
    // use a for loop to initialize each pin as an output:
    for (int thisPin = 0; thisPin < valveCount; thisPin++)
    {
      pinMode(moistureSensorPins[thisPin], INPUT);
      pinMode(valvePins[thisPin], OUTPUT);

      digitalWrite(valvePins[thisPin], HIGH);
    }
    digitalWrite(mainValvePin, HIGH);
    digitalWrite(moistureSensorActivationPin, LOW);

    pinMode(flowSensorPin, INPUT);
    digitalWrite(flowSensorPin, HIGH);

    // The Hall-effect sensor is connected to pin 2 which uses interrupt 0.
    // Configured to trigger on a FALLING state change (transition from HIGH
    // state to LOW state)
    attachInterrupt(flowSensorInterrupt, flowPulseCounter, FALLING);

  }

  void action()
  {  
      switch (programState)
      {
      case INITIALIZING:
  
        this->initializeAction();
        break;
  
      case ACTIVATING_MOISTURE_SENSORS:
  
  #ifdef WITH_SERIAL
        Serial.println("ACTIVATING MOISTURE SENSORS");
  #endif
  
        this->activateMoistSensorsAction();
        break;
  
      case READING_MOISTURE_SENSORS:
  
        this->readMoistSensorsAction();
  
        break;
  
      case INSPECTING_FOR_CHANGES:
      
        this->inspectForChangesAction();
        break;
  
      case ALERT:
      
        this->alertAction();
        break;
      }

  }

protected:

  void alertAction()
  {
    
#ifdef WITH_SERIAL
      Serial.print("ALERT : ");
      Serial.println(msg);
#endif
    
    digitalWrite(statusLed, HIGH);  // We have an active-low LED attached
    
    digitalWrite(mainValvePin, HIGH); // fermeture de la valve
    
    delay(2000);
    
    for (int thisPin = 0; thisPin < valveCount; thisPin++)
    {
      digitalWrite(valvePins[0], HIGH); // fermeture de la valve
    }    
    
    digitalWrite(moistureSensorActivationPin, LOW);
    
    // Disable the interrupt while calculating flow rate and sending the value to the host
    detachInterrupt(flowSensorInterrupt);
    
  }
  
  void initializeAction()
  {
    
#ifdef WITH_SERIAL
      Serial.println("INITIALIZING");
#endif

    digitalWrite(valvePins[0], LOW); // activation de la valve
    digitalWrite(valvePins[1], LOW); // activation de la valve
    delay(500);

    digitalWrite(valvePins[0], HIGH); // fermeture de la valve
    digitalWrite(valvePins[1], HIGH); // fermeture de la valve      
    delay(500);

    programState = ACTIVATING_MOISTURE_SENSORS;
  }


  void activateMoistSensorsAction()
  {      
    digitalWrite(moistureSensorActivationPin, HIGH);
    delay(50);

    programState = READING_MOISTURE_SENSORS;
  }

  void readMoistSensorsAction()
  {
    for (int thisPin = 0; thisPin < valveCount; thisPin++)
    {
      moistureSensorState[thisPin] = digitalRead(moistureSensorPins[thisPin]);
    }
    programState = INSPECTING_FOR_CHANGES;
  }

  void inspectForChangesAction()
  {      
    // détection de changement d'état du moisture sensor
    if (moistureSensorState[numValveToInspect] != prevMoistureSensorState[numValveToInspect]) 
    {
#ifdef WITH_SERIAL
      Serial.println(this->getMoistureSensorsState());
#endif

      changeValveState();
    }
    // détection de reprise suite à l'extinction d'une autre vanne
    if (valveState[numValveToInspect] != moistureSensorState[numValveToInspect]) 
    {
      changeValveState();
    }

    prevMoistureSensorState[numValveToInspect] = moistureSensorState[numValveToInspect];
    ++numValveToInspect;

    // on doit détecter les changements sur les autres vannes
    if (numValveToInspect < valveCount )
    {
      programState = INSPECTING_FOR_CHANGES;
      return;
    } 
    numValveToInspect = 0;

    // les vannes sont toutes fermées, et les plantes rassasiées. 
    if (getNbValvesOpened() == 0)
    {
      digitalWrite(moistureSensorActivationPin, LOW);

      #ifdef WITH_SERIAL
            Serial.println("SLEEPING FOR A FEW MINUTES");
      #endif

      programState = ACTIVATING_MOISTURE_SENSORS;
      // la valve est fermÃ©e, le tx d'humiditÃ© varie lentement, on peut allonger la durÃ©e entre deux mesures
      delay(5000);

      return;
    }            
    
     // l'eau coule et toutes les vannes ont été parcourues, on choisit un délai d'actualisation de la mesure court
    delay(1000);
    
    if (calcFlowStats() == -1)
      return;

    programState = READING_MOISTURE_SENSORS;
   
  }

  // Calcul des statistiques de consommation d'eau une fois une vanne ouverte
  // ATTENTION : on part du principe qu'une seule vanne est ouverte à la fois.
  int calcFlowStats() 
  {
    unsigned long newTime = millis();

      // Disable the interrupt while calculating flow rate and sending the value to
      // the host
      detachInterrupt(flowSensorInterrupt);

      // Because this loop may not complete in exactly 1 second intervals we calculate
      // the number of milliseconds that have passed since the last execution and use
      // that to scale the output. We also apply the flowSensorCalibrationFactor to scale the output
      // based on the number of pulses per second per units of measure (litres/minute in
      // this case) coming from the sensor.
      float flowRate = ((1000.0f / (newTime - flowStatsOldTime[numValveToInspect])) * flowPulseCount[numValveToInspect]) / flowSensorCalibrationFactor;
     
      // Divide the flow rate in litres/minute by 60 to determine how many litres have
      // passed through the sensor in this 1 second interval, then multiply by 1000 to
      // convert to millilitres.
      unsigned int flowMilliLitres = (flowRate / 60) * 1000;

      // Add the millilitres passed in this second to the cumulative total
      totalMilliLitres[numValveToInspect] += flowMilliLitres;

      // détection d'incohérences
      
      int diffMillilitres = totalMilliLitres[numValveToInspect] - lastTotalMilliLitres[numValveToInspect];
      
      if (diffMillilitres > 0)
      {
        // Note the time this processing pass was executed. Note that because we've
        // disabled interrupts the millis() function won't actually be incrementing right
        // at this point, but it will still return the value it was set to just before
        // interrupts went away.
        flowStatsOldTime[numValveToInspect] = newTime;        
        
        lastTotalMilliLitres[numValveToInspect] = totalMilliLitres[numValveToInspect];
      
      } else {

        if ( 15000 < (newTime - flowStatsOldTime[numValveToInspect] ) )
        {
          msg = "No flow";
          programState = ALERT;
          return -1;
        } 
      }
      
      // Reset the pulse counter so we can start incrementing again
      flowPulseCount[numValveToInspect] = 0;
  
 
      unsigned int frac;

#ifdef WITH_SERIAL  
      // Print the flow rate for this second in litres / minute
      Serial.print("Flow rate: ");
      Serial.print(int(flowRate));  // Print the integer part of the variable
      Serial.print(".");             // Print the decimal point
      // Determine the fractional part. The 10 multiplier gives us 1 decimal place.
      frac = (flowRate - int(flowRate)) * 10;
      Serial.print(frac, DEC) ;      // Print the fractional part of the variable
      Serial.print("L/min");
      // Print the number of litres flowed in this second
      Serial.print("  Current Liquid Flowing: ");             // Output separator
      Serial.print(flowMilliLitres);
      Serial.print("mL/Sec");

      // Print the cumulative total of litres flowed since starting
      Serial.print("  Output Liquid Quantity: ");             // Output separator
      Serial.print(totalMilliLitres[numValveToInspect]);
      Serial.println("mL"); 

#endif
      
      // Enable the interrupt again now that we've finished sending output
      attachInterrupt(flowSensorInterrupt, flowPulseCounter, FALLING);

    
    return 0;
  }

  void changeValveState()
  {       
    if (moistureSensorState[numValveToInspect]) // activation du moisture sensor
    {
      if (getNbValvesOpened() < maxValvesOpened && !valveState[numValveToInspect] )
      {
        programState = INSPECTING_FOR_CHANGES;

        msg = " => OPEN VALVE ";
        msg+= numValveToInspect;

        digitalWrite(valvePins[numValveToInspect], LOW); // activation de la valve
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
        digitalWrite(valvePins[numValveToInspect], HIGH);

        valveState[numValveToInspect] = 0;

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

  String getMoistureSensorsState()
  {
    String str;

    for (int ii = 0; ii< NB_VALVES; ++ii)
    {
      str+= moistureSensorState[ii];
      str += " "; 
    }    
    return str;
  }

public:
  /*
  Flow sensor Insterrupt Service Routine
   */
  void flowIncPulseCounter()
  {
    
    if (getNbValvesOpened() == 0 && programState != CLOSING_MAIN_VALVE)
    {
      if (lastIncoherentPulseCountTime == 0 || ((millis() - lastIncoherentPulseCountTime) > 60000))
      {
         lastIncoherentPulseCountTime = millis();
         incoherentPulseCount = 0;
                 digitalWrite(statusLed, LOW);  // We have an active-low LED attached
      }
      
      incoherentPulseCount++;
      
      if (incoherentPulseCount > 40)
      {
        msg = "water is flowing but valves are closed!"; 
        programState = ALERT;
         this->alertAction();
      }
       
    } 

    // Increment the pulse counter
    flowPulseCount[numValveToInspect]++;

  }


};



sprinkler valveMgr;

// the setup routine runs once when you press reset:
void setup()
{
#ifdef WITH_SERIAL
  // initialize serial communication at 9600 bits per second:
  Serial.begin(38400);
#endif

  pinMode(statusLed, OUTPUT);
  digitalWrite(statusLed, LOW);  // We have an active-low LED attached


}

// the loop routine runs over and over again forever:
void loop()
{        
  valveMgr.action();
}

/*
 Flow sensor Insterrupt Service Routine
 */
void flowPulseCounter()
{
  valveMgr.flowIncPulseCounter();

}


