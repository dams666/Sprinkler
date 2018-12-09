#include "main.h"
#include "menus.h"

#ifdef WITH_LOGGER
#include <SPI.h>
#include <SD.h>
#endif

#include "MOD_moisture_sensors.h"
#include "MOD_water_stats.h"
#include "MOD_valves.h"

#ifdef WITH_DS1307
#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#endif

// -------------------------------------------------------------------------------------------------

volatile byte                    __programState;
volatile byte                    __programNextState;
volatile unsigned long          __programStateMillis;
volatile unsigned long          __nextActionMillis;
volatile unsigned long          __actionMillis;

action_t array_actions[PRGM_STATE_NB]; 

String                 __msg;




chanConf               __channelConf[MAX_CHANNELS_];
MOD_moistureSensors_ * __MOD_moistureSensors;
MOD_waterStats_ *      __MOD_waterStats;
MOD_valves_ *          __MOD_valves;
GUI *                  __gui;

#ifdef WITH_LOGGER
SDLib::File             __fileLogger;
#endif

// variables locales
int                    __curChannel;
unsigned long          __nextTimeReadTimeMillis;
unsigned long          __curMillis;
// -------------------------------------------------------------------------------------------------


void globalFlowIncPulseCounter(){ __MOD_waterStats->flowIncPulseCounter(); }

void readChannelConfs()
{
  eeprom_read_bytes(  0, 
                      (byte*)__channelConf,
                      MAX_CHANNELS_ * sizeof(chanConf));
}

void writeChannelConfs()
{
  eeprom_write_bytes(  0, 
                      (const byte*)__channelConf,
                      MAX_CHANNELS_ * sizeof(chanConf));
}

int getNbChannelsActivated()
{
  int res = 0;
  for (int thisPin = 0; thisPin < MAX_CHANNELS_; ++thisPin)
    res += (int)__channelConf[thisPin].active;
  return res;
}

void setProgramAction(int state, unsigned int delay_)
{
  __programNextState = state;
  __nextActionMillis = millis() + delay_;
}

bool isAlertState()
{
  bool _alert = false;

  //désactivation de l'interrupt
  /*
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    _alert = __programState == PRGM_STATE_ALERT || __programNextState == PRGM_STATE_ALERT;
  }*/
  
  return __programState == PRGM_STATE_ALERT || __programNextState == PRGM_STATE_ALERT;
}



  static void initialiseAction() 
  {
    DEBUG_PRINTLN(F("--- INITIALIZING --- "));

    __MOD_valves->reset();
    __MOD_moistureSensors->reset();

    __gui->displayMenu(MAIN_MENU);

  }

  static void sleepAction()
  {
    // entrée dans l'état de sommeil
    if (__programStateMillis == __actionMillis)
    {
      __MOD_moistureSensors->reset();
      LCD_CLEAR();
      LCD_PRINT(0,3,F("SLEEPING..."));
    }
    
    Button_t button;
    // un bouton a été activé ?
    if ((button = __gui->readPushButton()) != BP_NONE)
    {
      setProgramAction(PRGM_STATE_INITIALIZING);
      return;
    }

    #ifdef WITH_DS1307
    // affichage de l'horloge
    if (__curMillis >= __nextTimeReadTimeMillis)
    { 
        char str[LCD_COLUMNS_ + 1];
      
        tmElements_t tm;
        if (RTC.read(tm))
        {
          sprintf_P(str, PSTR("%02d/%02d/%02d  %02d:%02d"), tm.Day, tm.Month, tm.Year, tm.Hour, tm.Minute);
          LCD_PRINT(0,0,str);
        }
                    
        __nextTimeReadTimeMillis = __curMillis + 60000;
    }
    #endif
    
    // sortie de l'état de sommeil
    if (__curMillis - __programStateMillis >= SLEEPING_DURATION_)
    {
      setProgramAction(PRGM_STATE_ACTIVATING_MOISTURE_SENSORS);
      return;
    }
        
    setProgramAction(PRGM_STATE_SLEEPING, 100);
  }
  
  static void alertAction()
  {
    if (!__MOD_valves->reset()) {
      __msg = F("Main valve KO!");
    }
    __MOD_moistureSensors->reset();

    while( __gui->readPushButton() == BP_NONE)
    {
      digitalWrite(BUZZER_PIN, HIGH);   // turn the LED on (HIGH is the voltage level)
      __gui->displayText(__msg, F("ALERT !"), false);
      delay(1000);
      
      digitalWrite(BUZZER_PIN, LOW);   // turn the LED on (HIGH is the voltage level)
      LCD_CLEAR();
      delay(1000);
    }

     setProgramAction(PRGM_STATE_INITIALIZING);
  }

  static void activateMoistSensAction()
  { 
    DEBUG_PRINTLN(F("--- ACTIVATING MOISTURE SENSORS ---"));
    __MOD_moistureSensors->reset();

    setProgramAction(PRGM_STATE_READING_MOISTURE_SENSORS, 100);
  }

  static void readMoistSensAction()
  {
    __MOD_moistureSensors->readValues();    
    setProgramAction(PRGM_STATE_INSPECTING_FOR_CHANGES); 
  }

  static void inspectForChangesAction()
  {          
    if (__channelConf[__curChannel].active)
    {
      // lancement / fermeture des vannes
      if ( __MOD_moistureSensors->hasStateChanged()) 
      {
        __MOD_valves->start();
        
      // détection de reprise suite à l'extinction d'une autre vanne
      } else if (__MOD_valves->state[__curChannel] != __MOD_moistureSensors->state[__curChannel]) 
      {
        __MOD_valves->start();
      }
      
      __MOD_moistureSensors->updateState();

      // l'eau est en train de couler, affichage des statistiques de conso
      if (__MOD_valves->state[__curChannel]) 
      {
        switch(__MOD_waterStats->calcFlow())
        {
          case WATER_OVERFLOW:
            __msg = F("\nValve ");
            __msg += (1 + __curChannel);
            __msg += F(" : Water \noverflow");
            
            setProgramAction(PRGM_STATE_ALERT);
            return;
          break;
          case WATER_BLOCKED:
            __msg = F("\nValve ");
            __msg += (1 + __curChannel);
            __msg += F(" : No water");
            setProgramAction(PRGM_STATE_ALERT);
            return;
          break;
          case WATER_FLOWING:
            __MOD_moistureSensors->show(1);
            __MOD_waterStats->printFlow();
          break;
        }
      }
    }

    ++__curChannel;

    // on doit détecter les changements sur les autres vannes
    if (__curChannel < MAX_CHANNELS_ )
    {
      setProgramAction(PRGM_STATE_INSPECTING_FOR_CHANGES, 50);
      return;
    } 
    __curChannel = 0;

    // les vannes sont toutes fermées, et les plantes rassasiées. 
    if (__MOD_valves->getNbValvesOpened() == 0)
    {
      setProgramAction(PRGM_STATE_SLEEPING);

      //TODO: mettre à jour les statistiques de conso dans l'eeprom ?
      return;
    }            

    // l'eau coule et toutes les vannes ont été parcourues, on choisit un délai d'actualisation de la mesure court
    setProgramAction(PRGM_STATE_READING_MOISTURE_SENSORS, 500);
  }



  void sprinklerInit()
  {
    // initialize serial communication at 9600 bits per second:
   #ifdef WITH_SERIAL
   Serial.begin(115200);
   #endif
  
    DEBUG_PRINTLN(F("--- STARTING SPRINKLER ---"));

    //------------------------------------------------------------------------------------ 
    // INIT GLOBAL VARS
    //------------------------------------------------------------------------------------
    __programStateMillis = 0;
    __programState = PRGM_STATE_UNDEFINED;
    __actionMillis = 0;

    array_actions[PRGM_STATE_UNDEFINED] = NULL;
    array_actions[PRGM_STATE_INITIALIZING] = &initialiseAction;
    array_actions[PRGM_STATE_SLEEPING] = &sleepAction;
    array_actions[PRGM_STATE_ACTIVATING_MOISTURE_SENSORS] = &activateMoistSensAction;
    array_actions[PRGM_STATE_READING_MOISTURE_SENSORS] = &readMoistSensAction;
    array_actions[PRGM_STATE_INSPECTING_FOR_CHANGES] = &inspectForChangesAction;
    array_actions[PRGM_STATE_ALERT] = &alertAction;

    __curMillis = 0;
    __curChannel = 0;
    __nextTimeReadTimeMillis = 0;
    //------------------------------------------------------------------------------------ 
    // INIT GUI
    //------------------------------------------------------------------------------------

    __gui = new GUI(LCD_I2C_ADDR, RC_PIN);

    __gui->centerText(F("SPRINKLER"));

    pinMode(BUZZER_PIN, OUTPUT);
    
    //------------------------------------------------------------------------------------ 
    // INIT MODULES
    //------------------------------------------------------------------------------------
    
    readChannelConfs();
    
    __MOD_moistureSensors = new MOD_moistureSensors_();
    __MOD_waterStats      = new MOD_waterStats_();
    __MOD_valves          = new MOD_valves_();
    
    //------------------------------------------------------------------------------------ 
    // INIT LOGGER
    //------------------------------------------------------------------------------------

    #ifdef WITH_LOGGER
    //__fileLogger = new SDLib::File();
    
    if (!SD.begin(4))
    {
      __msg = F("SD Cart:\nInit failed! ");
      setProgramAction(PRGM_STATE_ALERT);
      return;
    }
    #endif
    
    //------------------------------------------------------------------------------------ 
    // INIT TIME
    //------------------------------------------------------------------------------------

    #ifdef WITH_DS1307
      tmElements_t tm;
    
      if (!RTC.read(tm))
      {
        if (RTC.chipPresent())
        {
          __gui->displayMenu(CONFIGURE_MENU);
          
        } else {
    
          __msg = F("DS1307 read error!\nCheck circuitry");
          setProgramAction(PRGM_STATE_ALERT);
          return;
        }
      }
    #endif
    //------------------------------------------------------------------------------------ 
    // LAUNCH MENU
    //------------------------------------------------------------------------------------

    setProgramAction (PRGM_STATE_INITIALIZING, 0);
    
  }

void sprinklerAction()
{
  bool doAction = false;
  __curMillis = millis();

  // TODO : désactiver l'interrupt ? il est susceptible de modifier __nextActionMillis (mais peu mrobable)
  doAction = __curMillis >= __nextActionMillis;
   
  // on regarde si on effecture une nouvelle action
  if (!doAction)
    return;

  __actionMillis = __curMillis;
  
  if (__programState != __programNextState)
  {
      __programState = __programNextState;
      __programStateMillis = __curMillis;
  }

/*
  char buffer[LCD_COLUMNS_+1]; 
  String str = F("ACTION: \n");
  strcpy_P(buffer, PRGM_STATE_NAMES[__programState]);
  str += buffer;
  __gui->displayText( str);
  */
  // lancement de l'action
  array_actions[__programState]();
  
  //__gui->displayText( F("ACTION: \nDONE!"));
}
  

