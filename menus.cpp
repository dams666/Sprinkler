#include "main.h"
#include "menus.h"

#include "MOD_moisture_sensors.h"
#include "MOD_water_stats.h"
#include "MOD_valves.h"

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
          
        } else {
        
          __MOD_moistureSensors->reset();
          __MOD_waterStats->reset();
          __MOD_valves->reset();

          LCD_CLEAR();
          setState(PRGM_STATE_ACTIVATING_MOISTURE_SENSORS);
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

        __MOD_moistureSensors->setEnabled(true);
        do
        {
            __MOD_moistureSensors->readValues();
            __MOD_moistureSensors->show(1);

          delay(400);
        } while((__gui->readPushButton()) != BP_OK);

        __MOD_moistureSensors->setEnabled(false);
        
        LCD_CLEAR();
        __gui->centerText(F("STATISTICS"));
        __MOD_waterStats->show(1);
        delay(400);
        while((__gui->readPushButton()) != BP_OK){delay(400);}

        __gui->displayMenu(STATISTICS_MENU); 
    }
}


/** Affiche le choix de l'utilisateur */
void doConfigureMenuAction(byte selectedMenuItem)
{ 
     switch (selectedMenuItem) // See which menu item is selected and execute that correspond function
    {
      case 6:
      
        // restore defaults    

          for (int thisChan = 0; thisChan < MAX_CHANNELS_; thisChan++)
          {
            __channelStorage[thisChan].active = false;
            __channelStorage[thisChan].waterStatsStorage.maxMlPerSession = 1000;
            __channelStorage[thisChan].waterStatsStorage.nbWaterings = 0;
            __channelStorage[thisChan].waterStatsStorage.totalMililitres = 0;
          }

          writeChannelStorages();
          
          __gui->displayText(F("Factory settings \nrestored!"), NULL);
                  
        break;
      case 7:
        __gui->displayMenu(MAIN_MENU);    
        break;
      
      default:
        
        //----------------------------------------------------------------
        // Activation de la vanne
      
        __channelStorage[selectedMenuItem].active = __gui->displayYNPrompt(F("ACTIVATE ?"), __channelStorage[selectedMenuItem].active); 
          
        LCD_CLEAR();
      
        char str[80];
      
        if (__channelStorage[selectedMenuItem].active)
        {
          sprintf_P(str, PSTR("Channel %i is ON"), selectedMenuItem +1);
          __gui->displayText(str);
        
          //----------------------------------------------------------------
          // définition du max ml par session
          __channelStorage[selectedMenuItem].waterStatsStorage.maxMlPerSession = __gui->displayIntPrompt( F("Max ml per session:"), F("ml"),
                                                                                                          __channelStorage[selectedMenuItem].waterStatsStorage.maxMlPerSession, 
                                                                                                          0, 1000, 10);
        } else {
          sprintf_P(str, PSTR("Channel %i is OFF"), selectedMenuItem +1);
          __gui->displayText(str);
        }

        __curChannel = selectedMenuItem;  
        writeCurChannelStorage();
       
       setState(PRGM_STATE_INITIALIZING);
        
    }
  
}

 





