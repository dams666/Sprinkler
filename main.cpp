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
#include <Time.h>
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

int                             __curChannel;


chanConf               __channelStorage[MAX_CHANNELS_];
MOD_moistureSensors_ * __MOD_moistureSensors;
MOD_waterStats_ *      __MOD_waterStats;
MOD_valves_ *          __MOD_valves;
GUI *                  __gui;

#ifdef WITH_LOGGER
SDLib::File             __fileLogger;
#endif

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
  
  return __programState == PRGM_STATE_ALERT || __programNextState == PRGM_STATE_ALERT;;
}

String  print2digits(int number) {
  String s;
  if (number >= 0 && number < 10) {
    s = "0";
  }
  s+=number;
  return s;
}

String readTime()
{
  String s;

#ifdef WITH_DS1307
  tmElements_t tm;
  if (RTC.read(tm))
  {
    s = print2digits(tm.Day);
    s+= '/';
    s+= print2digits(tm.Month);
    s+= '/';
    s+=tmYearToCalendar(tm.Year);
    s+=' ';
    s+= print2digits(tm.Hour);
    s+=':';
    s+=print2digits(tm.Minute);
    s+=':';
    s+=print2digits(tm.Second);
  }
#else
  s = millis();
#endif
  return s;
}



  static void initialiseAction() 
  {
    DEBUG_PRINTLN(F("--- INITIALIZING --- "));

    __MOD_valves->closeAllValves();
    __MOD_moistureSensors->setEnabled(false);
        
    __gui->displayMenu(MAIN_MENU);

  }

  static void sleepAction()
  {
    // entrée dans l'état de sommeil
    if (__programStateMillis = __actionMillis)
    {
      __MOD_moistureSensors->setEnabled(false);

      __gui->displayText( F("\n\nSLEEPING..."), NULL, false);
    }
    
    Button_t button;
    // un bouton a été activé ?
    if ((button = __gui->readPushButton()) != BP_NONE)
    {
      setProgramAction(PRGM_STATE_INITIALIZING);
      return;
    }
    /*
    // affichage de l'horloge
    if (curMillis > __nextTimeReadTimeMillis)
    {
      LCD_PRINT(0,0,readTime());
                    
      __nextTimeReadTimeMillis = curMillis + 1000;
    }
    */
    
    // sortie de l'état de sommeil
    if (millis() - __programStateMillis > SLEEPING_DURATION_)
    {
      setProgramAction(PRGM_STATE_ACTIVATING_MOISTURE_SENSORS);
      return;
    }
        
    setProgramAction(PRGM_STATE_SLEEPING, 100);
  }
  
  static void alertAction()
  {
    if (!__MOD_valves->closeAllValves()) {
      __msg = F("Main valve KO!");
    }
    __MOD_moistureSensors->setEnabled(false);

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
    __MOD_moistureSensors->setEnabled(true);

    setProgramAction(PRGM_STATE_READING_MOISTURE_SENSORS, 100);
  }

  static void readMoistSensAction()
  {
    __MOD_moistureSensors->readValues();    
    setProgramAction(PRGM_STATE_INSPECTING_FOR_CHANGES); 
  }

  static void inspectForChangesAction()
  {          
    if (__channelStorage[__curChannel].active)
    {
      if ( __MOD_moistureSensors->hasStateChanged()) 
      {
        __MOD_valves->changeValveState();
        
      } else if (__MOD_valves->state[__curChannel] != __MOD_moistureSensors->state[__curChannel]) // détection de reprise suite à l'extinction d'une autre vanne
      {
        __MOD_valves->changeValveState();
      }
      
      __MOD_moistureSensors->updateState();
          
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
        
    __curChannel = 0;

    //------------------------------------------------------------------------------------ 
    // INIT GUI
    //------------------------------------------------------------------------------------

    __gui = new GUI(LCD_I2C_ADDR, RC_PIN);

    __gui->centerText(F("SPRINKLER"));

    pinMode(BUZZER_PIN, OUTPUT);
    
    //------------------------------------------------------------------------------------ 
    // INIT MODULES
    //------------------------------------------------------------------------------------
    
    readChannelStorages();
    
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
      __msg = F("DS1307 is stopped.\nRun SetTime");
      setProgramAction(PRGM_STATE_ALERT);
      return;
      
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
  unsigned long curMillis = millis();

  // TODO : désactiver l'interrupt ? il est susceptible de modifier __nextActionMillis (mais peu mrobable)
  doAction = curMillis >= __nextActionMillis;
   
  // on regarde si on effecture une nouvelle action
  if (!doAction)
    return;

  __actionMillis = curMillis;
  
  if (__programState != __programNextState)
  {
      __programState = __programNextState;
      __programStateMillis = curMillis;
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
  

