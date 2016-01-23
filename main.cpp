#include "main.h"
#include "MOD_moisture_sensors.h"
#include "MOD_water_stats.h"
#include "MOD_valves.h"

#include "menus.h"

// -------------------------------------------------------------------------------------------------

int                     __programState;
int                     __curChannel;
String *                __msg1;
String *                __msg2;
chanConf                __channelConfig[MAX_CHANNELS_];
MOD_moistureSensors_ *  __MOD_moistureSensors;
MOD_waterStats_ *       __MOD_waterStats;
MOD_valves_ *           __MOD_valves;
GUI *                   __gui;

// -------------------------------------------------------------------------------------------------




  int getNbChannelsActivated()
  {
    int res = 0;
    for (int thisPin = 0; thisPin < MAX_CHANNELS_; thisPin++)
      res += (int)__channelConfig[thisPin].active;
    return res;
  }
  
  void sprinklerInit()
  {
    // initialize serial communication at 9600 bits per second:
    Serial.begin(115200);
    //#endif
  
    DEBUG_PRINTLN("--- STARTING SPRINKLER ---");

    //------------------------------------------------------------------------------------ 
    // INIT GLOBAL VARS
    //------------------------------------------------------------------------------------

    // tout les canaux sont désactivés au démarrage
    // TODO : sauvegarder l'état dans l'EEPROM
    //memset(__channelConfig[__curChannel].active, 0, MAX_CHANNELS_);
    
    __programState = PRGM_STATE_INITIALIZING;
        
    __curChannel = 0;

    __msg1 = new String();
    __msg2 = new String();

    //------------------------------------------------------------------------------------ 
    // INIT GUI
    //------------------------------------------------------------------------------------
    
    __gui = new GUI();
    
    //------------------------------------------------------------------------------------ 
    // INIT MODULES
    //------------------------------------------------------------------------------------
    
    __MOD_moistureSensors = new MOD_moistureSensors_();
    __MOD_waterStats      = new MOD_waterStats_();
    __MOD_valves          = new MOD_valves_();
     
    //------------------------------------------------------------------------------------ 
    // INIT PHI_PROMPT LIBRARY
    //------------------------------------------------------------------------------------
    
    
    LCD_PRINT(0,0,">>>>SPRINKLER<<<<<<");

    __MOD_valves->purgeTransitionalCircuit();
    
    DEBUG_PRINTLN("INITIALIZATION : Done");

    delay(3000);

  }

  void alertAction()
  {
    __MOD_valves->closeAllValves();
    __MOD_moistureSensors->setEnabled(false);
    
    DEBUG_PRINT("ALERT : ");
    DEBUG_PRINTLN(*__msg1);

    while(true)
    {
      if (__gui->readPushButton() != BP_NONE){break;}
    
      LCD_CLEAR();
      LCD_PRINT(3,0, "=== ALERT ! ===");

      LCD_PRINT(0,2, *__msg1);
      if ((*__msg2) != "") LCD_PRINT(0,3, *__msg2);
      
      delay(1000);

          /* Attend l'appui sur un bouton */
    }

     __programState = PRGM_STATE_INITIALIZING;
  }





  void inspectForChangesAction()
  {      
    // détection de changement d'état du moisture sensor
    if (__channelConfig[__curChannel].active && __MOD_moistureSensors->state[__curChannel] != __MOD_moistureSensors->prevState[__curChannel]) 
    {
      __MOD_valves->changeValveState();
    }
    // détection de reprise suite à l'extinction d'une autre vanne
    if (__channelConfig[__curChannel].active && __MOD_valves->state[__curChannel] != __MOD_moistureSensors->state[__curChannel]) 
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
    if (__curChannel < MAX_CHANNELS_ )
    {
      __programState = PRGM_STATE_INSPECTING_FOR_CHANGES;
      return;
    } 
    __curChannel = 0;

    // les vannes sont toutes fermées, et les plantes rassasiées. 
    if (__MOD_valves->getNbValvesOpened() == 0)
    {
      __MOD_moistureSensors->setEnabled(false);

      LCD_CLEAR();
      LCD_PRINT(0,1, "ALL PLANTS OK !");
      LCD_PRINT(0,3, "SLEEPING...      ");
      
      DEBUG_PRINTLN("SLEEPING FOR A FEW MINUTES");

      //delay(5000);
      //__gui->lcd->off();

      
      __programState = PRGM_STATE_ACTIVATING_MOISTURE_SENSORS;
      // la valve est fermÃ©e, le tx d'humiditÃ© varie lentement, on peut allonger la durÃ©e entre deux mesures
      delay(SLEEPING_DURATION_);

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
          
        __gui->displayMenu(MAIN_MENU);
        
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

      case PRGM_STATE_ALERT:
        alertAction();
        break;
        
      //default:
      
      }

  }
  

