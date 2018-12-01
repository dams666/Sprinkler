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

int                     __programState;
int                     __programNextState;
unsigned long           __stateMillis;
unsigned long           __nextStateMillis;
int                     __curChannel;

unsigned long           __nextTimeReadTimeMillis;

String                  __msg;
chanConf                __channelStorage[MAX_CHANNELS_];
MOD_moistureSensors_ *  __MOD_moistureSensors;
MOD_waterStats_ *       __MOD_waterStats;
MOD_valves_ *           __MOD_valves;
GUI *                   __gui;

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

void setState(int state, unsigned int delay_)
{
  __programNextState = state;
  __nextStateMillis = millis() + delay_;
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



  void sprinklerInit()
  {
    // initialize serial communication at 9600 bits per second:
   #ifdef WITH_SERIAL
   Serial.begin(115200);
   #endif
  
    DEBUG_PRINTLN("--- STARTING SPRINKLER ---");

    //------------------------------------------------------------------------------------ 
    // INIT GLOBAL VARS
    //------------------------------------------------------------------------------------
    __stateMillis = 0;
    __programState = PRGM_STATE_UNDEFINED;
    
    setState (PRGM_STATE_INITIALIZING, 0);
        
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
      __msg = "SD Cart:\nInit failed! ";
      setState(PRGM_STATE_ALERT);
      return;
    }
    #endif
    
    //------------------------------------------------------------------------------------ 
    // INIT TIME
    //------------------------------------------------------------------------------------

    __nextTimeReadTimeMillis = 0;

#ifdef WITH_DS1307
  tmElements_t tm;

  if (!RTC.read(tm))
  {
    if (RTC.chipPresent())
    {
      __msg = "DS1307 is stopped.\nRun SetTime";
      setState(PRGM_STATE_ALERT);
      return;
      
    } else {

      __msg = "DS1307 read error!\nCheck circuitry";
      setState(PRGM_STATE_ALERT);
      return;
    }
  }
#endif
    
  }

  void alertAction()
  {
    __MOD_valves->closeAllValves();
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

     setState(PRGM_STATE_INITIALIZING);
  }


  void inspectForChangesAction()
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
            __msg = "\nValve ";
            __msg += (1 + __curChannel);
            __msg += " : Water \noverflow";
            
            setState(PRGM_STATE_ALERT);
            return;
          break;
          case WATER_BLOCKED:
            __msg = "\nValve ";
            __msg += (1 + __curChannel);
            __msg += " : No water";
            setState(PRGM_STATE_ALERT);
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
      setState(PRGM_STATE_INSPECTING_FOR_CHANGES);
      return;
    } 
    __curChannel = 0;

    // les vannes sont toutes fermées, et les plantes rassasiées. 
    if (__MOD_valves->getNbValvesOpened() == 0)
    {
      setState(PRGM_STATE_SLEEPING);
      return;
    }            

    // l'eau coule et toutes les vannes ont été parcourues, on choisit un délai d'actualisation de la mesure court
    setState(PRGM_STATE_READING_MOISTURE_SENSORS, 500);
  }



void sprinklerAction()
{
  bool hasStateChanged = false;
  unsigned long curMillis = millis();
  
    // on regarde si la tache suivante peut être exécutée
    if (curMillis < __nextStateMillis)
    {
      Button_t button;
      // un bouton a été activé ?
      if ((button = __gui->readPushButton()) != BP_NONE)
      {
        // si aucune vanne n'est ouverte, on peut interrompre la prochaine tâche et afficher le menu principal
        if (__programState != PRGM_STATE_ALERT)
        {
          setState(PRGM_STATE_INITIALIZING);
        }
      } else {
        delay(50);
      }
      return;
    
    } else {

        if (__programState != __programNextState)
        {
          hasStateChanged = true;  
          __programState = __programNextState;
          __stateMillis = millis();
        }
    }
  

  
  switch (__programState)
  {
      case PRGM_STATE_INITIALIZING:
        DEBUG_PRINTLN("--- INITIALIZING --- ");

        __MOD_valves->closeAllValves();
        __MOD_moistureSensors->setEnabled(false);
        
        __gui->displayMenu(MAIN_MENU);
        
        break;

      case PRGM_STATE_SLEEPING:

        if (hasStateChanged)
        {
          __MOD_moistureSensors->setEnabled(false);

          __gui->displayText( F("\n\nSLEEPING..."), NULL, false);
        }

        Button_t button;
        // un bouton a été activé ?
        if ((button = __gui->readPushButton()) != BP_NONE)
        {
          setState(PRGM_STATE_INITIALIZING);
        }

        if (curMillis > __nextTimeReadTimeMillis)
        {
            LCD_PRINT(0,0,readTime());
                    
            __nextTimeReadTimeMillis = curMillis + 1000;
        }
        
        if (curMillis - __stateMillis > SLEEPING_DURATION_)
        {
          setState(PRGM_STATE_ACTIVATING_MOISTURE_SENSORS);
        }
        
        break;
      case PRGM_STATE_ACTIVATING_MOISTURE_SENSORS:
 
        DEBUG_PRINTLN("--- ACTIVATING MOISTURE SENSORS ---");
   
        __MOD_moistureSensors->setEnabled(true);

        setState(PRGM_STATE_READING_MOISTURE_SENSORS, 100);
    
        break;
  
      case PRGM_STATE_READING_MOISTURE_SENSORS:
  
        __MOD_moistureSensors->readValues();    
        
        setState(PRGM_STATE_INSPECTING_FOR_CHANGES);
  
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
  

