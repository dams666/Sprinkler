#include "main.h"
#include "MOD_moisture_sensors.h"
#include "MOD_water_stats.h"
#include "MOD_valves.h"

#define SLEEPING_DURATION         60000
#define NB_VALVES                 2

int __programState;
int __eeAddress;
int __nbChannels;
int __curChannel;

String *__msg1;
String *__msg2;

LiquidCrystal_I2C * __LCD;

MOD_moistureSensors_ *  __MOD_moistureSensors;
MOD_waterStats_ *       __MOD_waterStats;
MOD_valves_ *           __MOD_valves;


  void sprinklerInit()
  {
    // initialize serial communication at 9600 bits per second:
    Serial.begin(115200);
    //#endif
  
    DEBUG_PRINTLN("--- STARTING SPRINKLER ---");

    //------------------------------------------------------------------------------------ 
    // INIT GLOBAL VARS
    //------------------------------------------------------------------------------------
    
    __programState = PRGM_STATE_INITIALIZING;
        
    __eeAddress = 0;

    __nbChannels = NB_VALVES;
    __curChannel = 0;

    __msg1 = new String();
    __msg2 = new String();
    
    __LCD = new LiquidCrystal_I2C (0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

    __MOD_moistureSensors = new MOD_moistureSensors_();
    __MOD_waterStats      = new MOD_waterStats_();
    __MOD_valves          = new MOD_valves_();

    __LCD->begin(20,4);         // initialize the __LCD for 20 chars 4 lines

    LCD_PRINT(2,0,"=== SPRINKLER ===");
    
    String s;
    s = "-Nb valves: ";
    s += __nbChannels;
    
    LCD_PRINT(0,1, s);
    delay(2000);
  
    DEBUG_PRINTLN("INITIALIZATION : Done");

    delay(500);
  }

  void alertAction()
  {
    __MOD_valves->closeAllValves();
        
    DEBUG_PRINT("ALERT : ");
    DEBUG_PRINTLN(*__msg1);
    
    while(true)
    {
      __LCD->clear();
      delay(100);    
      LCD_PRINT(3,0, "=== ALERT ! ===");

      LCD_PRINT(0,2, *__msg1);
      if ((*__msg2) != "") LCD_PRINT(0,3, *__msg2);
      
      delay(1000);
    }
  }

  void inspectForChangesAction()
  {      
    // détection de changement d'état du moisture sensor
    if (__MOD_moistureSensors->state[__curChannel] != __MOD_moistureSensors->prevState[__curChannel]) 
    {
      __MOD_valves->changeValveState();
    }
    // détection de reprise suite à l'extinction d'une autre vanne
    if (__MOD_valves->state[__curChannel] != __MOD_moistureSensors->state[__curChannel]) 
    {
      __MOD_valves->changeValveState();
    }

    if (__MOD_valves->state[__curChannel]) 
    {
      switch(__MOD_waterStats->calcFlow())
      {
        case WATER_OVERFLOW:
          (*__msg1) = "Valve ";
          (*__msg1) += (1 + __curChannel);
          (*__msg1) += " : Water";
          (*__msg2) = "overflow";
          
          __programState = PRGM_STATE_ALERT;
          return;
        break;
        case WATER_BLOCKED:
          (*__msg1) = "Valve ";
          (*__msg1) += (1 + __curChannel);
          (*__msg1) += " : No water";
          (*__msg2) = "";
          __programState = PRGM_STATE_ALERT;
          return;
        break;
        case WATER_FLOWING:
          __MOD_waterStats->printFlow();
        break;
      }
    }

    __MOD_moistureSensors->prevState[__curChannel] = __MOD_moistureSensors->state[__curChannel];
    ++__curChannel;

    // on doit détecter les changements sur les autres vannes
    if (__curChannel < __nbChannels )
    {
      __programState = PRGM_STATE_INSPECTING_FOR_CHANGES;
      return;
    } 
    __curChannel = 0;

    // les vannes sont toutes fermées, et les plantes rassasiées. 
    if (__MOD_valves->getNbValvesOpened() == 0)
    {
      __MOD_moistureSensors->setEnabled(false);

      LCD_PRINT(0,3, "SLEEPING...      ");
      
      DEBUG_PRINTLN("SLEEPING FOR A FEW MINUTES");
      
      __programState = PRGM_STATE_ACTIVATING_MOISTURE_SENSORS;
      // la valve est fermÃ©e, le tx d'humiditÃ© varie lentement, on peut allonger la durÃ©e entre deux mesures
      delay(SLEEPING_DURATION);

      return;
    }            

    __programState = PRGM_STATE_READING_MOISTURE_SENSORS;
    
     // l'eau coule et toutes les vannes ont été parcourues, on choisit un délai d'actualisation de la mesure court
    delay(1000);
   
  }
 
void sprinklerAction()
{
  switch (__programState)
      {
      case PRGM_STATE_INITIALIZING:
        DEBUG_PRINTLN("--- INITIALIZING --- ");
          
        __MOD_valves->purgeTransitionalCircuit();

        __programState = PRGM_STATE_ACTIVATING_MOISTURE_SENSORS;
        break;
  
      case PRGM_STATE_ACTIVATING_MOISTURE_SENSORS:
 
        DEBUG_PRINTLN("--- ACTIVATING MOISTURE SENSORS ---");
   
        __MOD_moistureSensors->setEnabled(true);

        __programState = PRGM_STATE_READING_MOISTURE_SENSORS;
    
        break;
  
      case PRGM_STATE_READING_MOISTURE_SENSORS:
  
        __MOD_moistureSensors->readValues();
        
        LCD_PRINT(0,2,__MOD_moistureSensors->getState());
        
        __programState = PRGM_STATE_INSPECTING_FOR_CHANGES;
  
        break;
  
      case PRGM_STATE_INSPECTING_FOR_CHANGES:
      
        inspectForChangesAction();
        break;
  
      default:
      
        alertAction();
        break;
      }

  }
  

