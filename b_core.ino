
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



  void initSprinkler()
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

    // The hall-effect flow sensor outputs approximately 60 pulses per second per
    // litre/minute of flow.
    flowSensorCalibrationFactor = 60.0f;

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

      digitalWrite(valvePins[thisPin], RELAY_OFF);
    }
    digitalWrite(mainValvePin, HIGH);
    digitalWrite(moistureSensorActivationPin, LOW);

    pinMode(flowSensorPin, INPUT);
    digitalWrite(flowSensorPin, HIGH);

    // The Hall-effect sensor is connected to pin 2 which uses interrupt 0.
    // Configured to trigger on a FALLING state change (transition from HIGH
    // state to LOW state)
    attachInterrupt(flowSensorInterrupt, flowIncPulseCounter, FALLING);

  }

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
      digitalWrite(valvePins[thisPin], RELAY_OFF); // fermeture de la valve
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

    digitalWrite(valvePins[0], RELAY_ON); // activation de la valve
    digitalWrite(valvePins[1], RELAY_ON); // activation de la valve
    delay(500);

    digitalWrite(valvePins[0], RELAY_OFF); // fermeture de la valve
    digitalWrite(valvePins[1], RELAY_OFF); // fermeture de la valve      
    delay(500);

    programState = ACTIVATING_MOISTURE_SENSORS;
  }

  void inspectForChangesAction()
  {      
    // détection de changement d'état du moisture sensor
    if (moistureSensorState[numValveToInspect] != prevMoistureSensorState[numValveToInspect]) 
    {
#ifdef WITH_SERIAL
      Serial.println(getMoistureSensorsState());
#endif

      changeValveState();
    }
    // détection de reprise suite à l'extinction d'une autre vanne
    if (valveState[numValveToInspect] != moistureSensorState[numValveToInspect]) 
    {
      changeValveState();
    }

    if (valveState[numValveToInspect]) 
    {
      if (calcFlowStats() == -1)
        return;
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
    
    programState = READING_MOISTURE_SENSORS;
   
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





