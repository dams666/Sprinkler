
#undef WITH_SERIAL

#define NB_VALVES 6

class valveManager
{
protected:

// VALVES VARIABLES

int mainValvePin;

int valvePins[NB_VALVES];
int valveCount;

int valveState[NB_VALVES];

int numValveToInspect; // identifiant courant de valve à inspecter ( pour l'état MANAGING_VALVES )

int maxValvesOpened;

// MOISTURE SENSORS

int moistureSensorPins[NB_VALVES];

int moistureSensorState[NB_VALVES];        // current moistureSensorState of the machine
int prevMoistureSensorState[NB_VALVES];         // previous moistureSensorState of the machine

int moistureSensorActivationPin;

// PROGRAM STATE

enum  programStates {
  INITIALIZING,
  ACTIVATING_MOISTURE_SENSORS,
  READING_MOISTURE_SENSORS,
  MANAGING_VALVES
} programState;


String msg;

public:

valveManager()
{
  // PINS
  
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
   
  // STATES
  
  programState = INITIALIZING;
  
  for (int ii = 0; ii< NB_VALVES; ++ii)
  {
    valveState[ii] = 0;
    moistureSensorState[ii] = 0;
    prevMoistureSensorState[ii] = 0;
  }
 
  valveCount = 5; 
 
  numValveToInspect = 0;
  
  maxValvesOpened = 2;

  // INIT PINS
  
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
  
}

void action()
{  
  switch (programState)
  {
    case INITIALIZING:
      
      #ifdef WITH_SERIAL
        Serial.println("INITIALIZING");
      #endif
     
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
    
    case MANAGING_VALVES:
            
      this->manageValvesAction();
      break;
  }
}

protected:

void initializeAction()
{
  digitalWrite(valvePins[0], LOW); // activation de la valve
  digitalWrite(valvePins[1], LOW); // activation de la valve
  delay(500);
      
  digitalWrite(valvePins[0], HIGH); // activation de la valve
  digitalWrite(valvePins[1], HIGH); // activation de la valve      
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
      programState = MANAGING_VALVES;
 }
 
void manageValvesAction()
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
  
  programState = MANAGING_VALVES;
  
  // si on doit repasser en mode mesure
  if (numValveToInspect >= valveCount )
  {  
    numValveToInspect = 0;
        
    // si l'eau coule, on choisit un dé©lai d'actualisation de la mesure court
    if (getNbValvesOpened() > 0)
    {
      programState = READING_MOISTURE_SENSORS;
          
      delay(200);
      
    } else {
          
        digitalWrite(moistureSensorActivationPin, LOW);

        #ifdef WITH_SERIAL
        Serial.println("SLEEPING FOR A FEW MINUTES");
        #endif
          
        programState = ACTIVATING_MOISTURE_SENSORS;
        // la valve est fermÃ©e, le tx d'humiditÃ© varie lentement, on peut allonger la durÃ©e entre deux mesures
        delay(60000);
      }
    }
 }
 
 void changeValveState()
 {       
      if (moistureSensorState[numValveToInspect]) // activation du moisture sensor
      {
        if (getNbValvesOpened() < maxValvesOpened && !valveState[numValveToInspect] )
        {
          msg = " => OPEN VALVE ";
          msg+= numValveToInspect;
              
          digitalWrite(valvePins[numValveToInspect], LOW); // activation de la valve
          valveState[numValveToInspect] = 1;
  
          #ifdef WITH_SERIAL  
          Serial.println(msg);
          #endif
              
          delay(200);
              
          digitalWrite(mainValvePin, LOW); // activation de la valve principale
        }
      } else { // fermeture du moisture sensor
          
        if (valveState[numValveToInspect]) 
        {
          msg = " => CLOSE VALVE ";
          msg+= numValveToInspect;
          
          if (getNbValvesOpened() == 1)
          {
            digitalWrite(mainValvePin, HIGH); // fermeture de la vanne principale
            delay(2000); // on lasse le temps a l'eau de s'écouler, et de faire baisser la pression dans les tuyaux
          }
             
          // fermeture de la vanne         
          digitalWrite(valvePins[numValveToInspect], HIGH);
              
          valveState[numValveToInspect] = 0;
              
          #ifdef WITH_SERIAL  
          Serial.println(msg);
          #endif
              
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

  

};
 


valveManager valveMgr;

// the setup routine runs once when you press reset:
void setup()
{
  #ifdef WITH_SERIAL
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  #endif

}

// the loop routine runs over and over again forever:
void loop()
{        
  valveMgr.action();
}



