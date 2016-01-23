#include "main.h"
#include "menus.h"

#include "MOD_moisture_sensors.h"
#include "MOD_water_stats.h"
#include "MOD_valves.h"

// -------------------------------------------------------------------------------------------------

int                     __programState;
unsigned long           __nextStateMillis;
int                     __curChannel;
String *                __msg;
chanConf                __channelStorage[MAX_CHANNELS_];
MOD_moistureSensors_ *  __MOD_moistureSensors;
MOD_waterStats_ *       __MOD_waterStats;
MOD_valves_ *           __MOD_valves;
GUI *                   __gui;

// -------------------------------------------------------------------------------------------------


void readChannelStorages()
{
  eeprom_read_bytes(  0, 
                      (byte*)__channelStorage,
                      MAX_CHANNELS_ * sizeof(chanConf));
}

void writeChannelStorages()
{
  eeprom_write_bytes(  0, 
                      (const byte*)__channelStorage,
                      MAX_CHANNELS_ * sizeof(chanConf));
}

void readCurChannelStorage()
{
   eeprom_read_bytes( sizeof(chanConf) * __curChannel,
                      (byte*)(__channelStorage + __curChannel),
                      sizeof(chanConf)); 
}

void writeCurChannelStorage()
{
  eeprom_write_bytes( sizeof(chanConf) * __curChannel, 
                      (const byte*)(__channelStorage + __curChannel), 
                      sizeof(chanConf)); 
}

int getNbChannelsActivated()
{
  int res = 0;
  for (int thisPin = 0; thisPin < MAX_CHANNELS_; thisPin++)
    res += (int)__channelStorage[thisPin].active;
  return res;
}

void setNextState(int state, unsigned int delay_)
{
  __programState = state;
  __nextStateMillis = millis() + delay_;
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
        
    setNextState (PRGM_STATE_INITIALIZING);
        
    __curChannel = 0;

    __msg = new String();

    //------------------------------------------------------------------------------------ 
    // INIT GUI
    //------------------------------------------------------------------------------------
    
    __gui = new GUI();
    
    //------------------------------------------------------------------------------------ 
    // INIT MODULES
    //------------------------------------------------------------------------------------

    // tout les canaux sont désactivés au démarrage
    // TODO : sauvegarder l'état dans l'EEPROM
    //eeprom_erase_all(0);

    readChannelStorages();
    
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

    while( __gui->readPushButton() == BP_NONE)
    {
      __gui->displayText(*__msg, "ALERT !", false);
      
      delay(1000);
      LCD_CLEAR();
      delay(1000);
    }

     setNextState(PRGM_STATE_INITIALIZING);
  }

  void inspectForChangesAction()
  {      
    // détection de changement d'état du moisture sensor
    if (__channelStorage[__curChannel].active && __MOD_moistureSensors->state[__curChannel] != __MOD_moistureSensors->prevState[__curChannel]) 
    {
      __MOD_valves->changeValveState();
    }
    // détection de reprise suite à l'extinction d'une autre vanne
    if (__channelStorage[__curChannel].active && __MOD_valves->state[__curChannel] != __MOD_moistureSensors->state[__curChannel]) 
    {
      __MOD_valves->changeValveState();
    }

    if (__MOD_valves->state[__curChannel]) 
    {
      switch(__MOD_waterStats->calcFlow())
      {
        case WATER_OVERFLOW:
          (*__msg) = "\nValve ";
          (*__msg) += (1 + __curChannel);
          (*__msg) += " : Water \noverflow";
          
          setNextState(PRGM_STATE_ALERT);
          return;
        break;
        case WATER_BLOCKED:
          (*__msg) = "\nValve ";
          (*__msg) += (1 + __curChannel);
          (*__msg) += " : No water";
          setNextState(PRGM_STATE_ALERT);
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
      setNextState(PRGM_STATE_INSPECTING_FOR_CHANGES);
      return;
    } 
    __curChannel = 0;

    // les vannes sont toutes fermées, et les plantes rassasiées. 
    if (__MOD_valves->getNbValvesOpened() == 0)
    {
      __MOD_moistureSensors->setEnabled(false);

      __gui->displayText( "ALL PLANTS OK !\n\nSLEEPING...", "", false);

      setNextState(PRGM_STATE_ACTIVATING_MOISTURE_SENSORS, SLEEPING_DURATION_);
      return;
    }            

    // l'eau coule et toutes les vannes ont été parcourues, on choisit un délai d'actualisation de la mesure court
    setNextState(PRGM_STATE_READING_MOISTURE_SENSORS, 1000);
    
  }


 
void sprinklerAction()
{
  // on regarde si la tache suivante peut être exécutée
  if (millis() < __nextStateMillis)
  {
    Button_t button;
    // un bouton a été activé ?
    if ((button = __gui->readPushButton()) != BP_NONE)
    {
      // si aucune vanne n'est ouverte, on peut interrompre la prochaine tâche et afficher le menu principal
      if (__MOD_valves->getNbValvesOpened() == 0  && __programState != PRGM_STATE_ALERT)
      {
        setNextState(PRGM_STATE_INITIALIZING);
      }
    } else {
      delay(50);
    }
    return;
  }
  
  switch (__programState)
  {
      case PRGM_STATE_INITIALIZING:
        DEBUG_PRINTLN("--- INITIALIZING --- ");
          
        __gui->displayMenu(MAIN_MENU);
        
        break;
      case PRGM_STATE_ACTIVATING_MOISTURE_SENSORS:
 
        DEBUG_PRINTLN("--- ACTIVATING MOISTURE SENSORS ---");
   
        __MOD_moistureSensors->setEnabled(true);

        setNextState(PRGM_STATE_READING_MOISTURE_SENSORS);
    
        break;
  
      case PRGM_STATE_READING_MOISTURE_SENSORS:
  
        __MOD_moistureSensors->readValues();
        
        LCD_PRINT(0,2,__MOD_moistureSensors->getState());
        
        setNextState(PRGM_STATE_INSPECTING_FOR_CHANGES);
  
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
  

