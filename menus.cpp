#include "main.h"
#include "menus.h"

#include "MOD_moisture_sensors.h"
#include "MOD_water_stats.h"
#include "MOD_valves.h"

#ifdef WITH_DS1307
#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#endif

/** Affiche le choix de l'utilisateur */
void doMainMenuAction(byte selectedMenuItem)
{ 
     switch (selectedMenuItem) // See which menu item is selected and execute that correspond function
    {
      case 0:
        if (getNbChannelsActivated() == 0)
        {
          __gui->displayText( F("All channels are OFF"));
          __gui->displayMenu(MAIN_MENU);

          setProgramAction(PRGM_STATE_INITIALIZING);
          
        } else {
          __MOD_moistureSensors->reset();
          __MOD_waterStats->reset();
          __MOD_valves->reset();

          setProgramAction(PRGM_STATE_ACTIVATING_MOISTURE_SENSORS);
        }
        break;
        case 1:
        __gui->displayMenu(STATISTICS_MENU); 
        break;   
      case 2:
        __gui->displayMenu(CONFIGURE_MENU); 
        break;    
    } 
}

/** Affiche le choix de l'utilisateur */
void doStatisticsMenuAction(byte selectedMenuItem)
{ 
     switch (selectedMenuItem) // See which menu item is selected and execute that correspond function
    {
      case 6:
        __gui->displayMenu(MAIN_MENU);    
        break;
      
      default:
        __curChannel = selectedMenuItem;

        LCD_CLEAR();
        __gui->centerText(F("STATISTICS"));

        __MOD_moistureSensors->start();
        do
        {
            __MOD_moistureSensors->readValues();
            __MOD_moistureSensors->show(1);

          delay(400);
        } while((__gui->readPushButton()) != BP_OK);

        __MOD_moistureSensors->reset();
        
        LCD_CLEAR();

        waterStatsChanStorage_ waterStats;
        char text[20][LCD_COLUMNS_+1];
        int txtArrLen = 0;
         
        __MOD_waterStats->readLogStats(text, txtArrLen, waterStats);
        
        __gui->centerText(F("STATISTICS"));
        __MOD_waterStats->show(1);
        char str[80];
        sprintf_P(str, PSTR("-Total: %d ml"), waterStats.totalMililitres);
        LCD_PRINT(0,2, str);
        
        while((__gui->readPushButton()) != BP_OK){delay(200);}
          
        __gui->displayText2(text,txtArrLen, F("LOG ")); 
        
        __gui->displayMenu(STATISTICS_MENU); 
    }
}


/** Affiche le choix de l'utilisateur */
void doConfigureMenuAction(byte selectedMenuItem)
{ 
     switch (selectedMenuItem) // See which menu item is selected and execute that correspond function
    {
      case 6:
        // Date / Time
        #ifdef WITH_DS1307
        tmElements_t tm;

        tm.Year = __gui->displayIntPrompt( F("Year:"), F(""), 2018, 2018, 2050, 1);
        tm.Month = __gui->displayIntPrompt( F("Month:"), F(""), 0, 1, 12, 1);
        tm.Day = __gui->displayIntPrompt( F("Day:"), F(""), 0, 1, 31, 1);
        
        tm.Hour = __gui->displayIntPrompt( F("Hours:"), F("h"), 0, 0, 24, 1);
        tm.Minute = __gui->displayIntPrompt( F("Minutes:"), F("m"), 0, 0, 60, 1);
        tm.Second = 0;
        
        RTC.write(tm);

        __gui->displayText(F("Date/Time set!"), NULL);
        #else
        __gui->displayText(F("Date/Time disabled!"), NULL);
        #endif
        
        break;
      case 7:
        // restore defaults    
          /*
          for (int thisChan = 0; thisChan < MAX_CHANNELS_; thisChan++)
          {
            __channelConf[thisChan].active = false;
            __channelConf[thisChan].waterStatsStorage.maxMlPerSession = 1000;
            __channelConf[thisChan].waterStatsStorage.nbWaterings = 0;
            __channelConf[thisChan].waterStatsStorage.totalMililitres = 0;
          }

          writeChannelConfs();
          */

          __gui->displayText(F("Clearing memory..."), F("FACTORY SETTINGS"), false);
          
          eeprom_erase_all(0);
          
          __gui->displayText(F("Factory settings \nrestored!"), NULL);
                  
        break;
      case 8:
        __gui->displayMenu(MAIN_MENU);    
        break;
      
      default:
        
        //----------------------------------------------------------------
        // Activation de la vanne
      
        __channelConf[selectedMenuItem].active = __gui->displayYNPrompt(F("ACTIVATE ?"), __channelConf[selectedMenuItem].active); 
          
        LCD_CLEAR();
      
        char str[80];
      
        if (__channelConf[selectedMenuItem].active)
        {
          sprintf_P(str, PSTR("Channel %i is ON"), selectedMenuItem +1);
          __gui->displayText(str);
        
          //----------------------------------------------------------------
          // définition du max ml par session
          __channelConf[selectedMenuItem].maxMlPerSession = __gui->displayIntPrompt( F("Max ml per session:"), F("ml"),
                                                                                                          __channelConf[selectedMenuItem].maxMlPerSession, 
                                                                                                          0, 1000, 10);
        } else {
          sprintf_P(str, PSTR("Channel %i is OFF"), selectedMenuItem +1);
          __gui->displayText(str);
        }

        __curChannel = selectedMenuItem;  
        writeChannelConfs();
       
       setProgramAction(PRGM_STATE_INITIALIZING);
        
    }
  
}

 





