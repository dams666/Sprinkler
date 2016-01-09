  
#define NB_VALVES                 1
#define MAX_LILILITRES_PER_VALVE  1000
#define SLEEPING_DURATION         60000

  enum  waterFlow {
    WATER_FLOWING,
    WATER_STOPPED,
    WATER_OVERFLOW
  } ;
  
  //------------------------------------------------------------------------------------
  // PINS
  //------------------------------------------------------------------------------------

  int mainValvePin;

  int valvePins[NB_VALVES];
  int fertilizerValvePin;
  
  int moistureSensorPins[NB_VALVES];

  int moistureSensorActivationPin;

  byte flowSensorInterrupt;
  byte flowSensorPin;

  //------------------------------------------------------------------------------------
  // VALVES VARIABLES
  //------------------------------------------------------------------------------------

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
  volatile byte flowPulseCount;  

  unsigned long totalMililitresSession[NB_VALVES];
  unsigned long lastTotalMililitresSession[NB_VALVES];

  unsigned long totalMililitres[NB_VALVES];
  unsigned long nbWaterings[NB_VALVES];
  
  unsigned long flowStatsOldTime;
  
  int incoherentPulseCount;
  unsigned long lastIncoherentPulseCountTime;
  
  //------------------------------------------------------------------------------------
  // OTHER
  //------------------------------------------------------------------------------------

  String msg;

  void initSprinkler()
  {
    DEBUG_PRINTLN("--- STARTING SPRINKLER ---");
    
    lcd.setCursor(2,0); //Start at character 4 on line 0
    lcd.print("=== SPRINKLER ===");
    lcd.setCursor(0,2);
    lcd.print("Initializing...");

    //------------------------------------------------------------------------------------ 
    // INIT MODULES
    //------------------------------------------------------------------------------------

    initValves();
    initMoistureSensors();
    initWaterStats();

    DEBUG_PRINTLN("INITIALIZATION : Done");
   
    lcd.setCursor(15,2); //Start at character 4 on line 0
    lcd.print("Done");
    
    programState = INITIALIZING;

    delay(2000);
  }

  void alertAction()
  {
    closeAllValves();
        
    DEBUG_PRINT("ALERT : ");
    DEBUG_PRINTLN(msg);
    
    while(true)
    {
      lcd.clear();
      delay(100);    
      lcd.setCursor(3,0); //Start at character 4 on line 0
      lcd.print("=== ALERT ! ===");
      lcd.setCursor(0,2); //Start at character 4 on line 0
      lcd.print(msg);
      delay(1000);
    }
  }
  
  void initializeAction()
  {
    purgeTransitionalCircuit();

    programState = ACTIVATING_MOISTURE_SENSORS;
  }

  void inspectForChangesAction()
  {      
    // détection de changement d'état du moisture sensor
    if (moistureSensorState[numValveToInspect] != prevMoistureSensorState[numValveToInspect]) 
    {
      //DEBUG_PRINTLN(getMoistureSensorsState());

      changeValveState();
    }
    // détection de reprise suite à l'extinction d'une autre vanne
    if (valveState[numValveToInspect] != moistureSensorState[numValveToInspect]) 
    {
      changeValveState();
    }

    if (valveState[numValveToInspect]) 
    {
      switch(calcFlowStats())
      {
        case WATER_OVERFLOW:
          msg = "Too much water for valve ";
          msg+= numValveToInspect;
          programState = ALERT;
          return;
        break;
        case WATER_STOPPED:
          if ( 4000 < (millis() - flowStatsOldTime ))
          {
            msg = "No water for valve ";
            msg+= numValveToInspect;
            programState = ALERT;
            return;
          }
        break;
        case WATER_FLOWING:
          lcd.setCursor(0,3);
          String s;
          s = "Flow: ";
          s+= totalMililitresSession[numValveToInspect];
          s+= " ml";
          lcd.print(s);
      }
    }

    prevMoistureSensorState[numValveToInspect] = moistureSensorState[numValveToInspect];
    ++numValveToInspect;

    // on doit détecter les changements sur les autres vannes
    if (numValveToInspect < NB_VALVES )
    {
      programState = INSPECTING_FOR_CHANGES;
      return;
    } 
    numValveToInspect = 0;

    // les vannes sont toutes fermées, et les plantes rassasiées. 
    if (getNbValvesOpened() == 0)
    {
      digitalWrite(moistureSensorActivationPin, LOW);

      delay(1000);
      lcd.setCursor(0,3); //Start at character 4 on line 0
      lcd.print("SLEEPING...");
      
      DEBUG_PRINTLN("SLEEPING FOR A FEW MINUTES");
      
      programState = ACTIVATING_MOISTURE_SENSORS;
      // la valve est fermÃ©e, le tx d'humiditÃ© varie lentement, on peut allonger la durÃ©e entre deux mesures
      delay(SLEEPING_DURATION);

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
    for (int thisPin = 0; thisPin < NB_VALVES; thisPin++)
    {
      moistureSensorState[thisPin] = digitalRead(moistureSensorPins[thisPin]);
    }
    programState = INSPECTING_FOR_CHANGES;
  }



