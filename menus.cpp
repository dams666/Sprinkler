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
          __gui->displayText("All channels are OFF");

        } else {

          __MOD_moistureSensors->reset();
          __MOD_waterStats->reset();
          __MOD_valves->reset();
          
          setNextState(PRGM_STATE_ACTIVATING_MOISTURE_SENSORS);
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
      case 7:
        __gui->displayMenu(MAIN_MENU);    
        break;
      
      default:
        __curChannel = selectedMenuItem;
        __MOD_waterStats->show();
        while((__gui->readPushButton()) == BP_NONE){}
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
          
          __gui->displayText("Factory settings \nrestored!");
                  
        break;
      case 7:
        __gui->displayMenu(MAIN_MENU);    
        break;
      
      default:
        
        //----------------------------------------------------------------
        // Activation de la vanne
      
        __channelStorage[selectedMenuItem].active = __gui->displayYNPrompt("ACTIVATE ?", __channelStorage[selectedMenuItem].active); 
          
        LCD_CLEAR();
      
        char str[80];
      
        if (__channelStorage[selectedMenuItem].active)
        {
          sprintf(str, "Channel %i is ON", selectedMenuItem +1);
        } else {
          sprintf(str, "Channel %i is OFF", selectedMenuItem +1);
        }

        __gui->displayText(str);
        
        //----------------------------------------------------------------
        // définition du max ml par session
        __channelStorage[selectedMenuItem].waterStatsStorage.maxMlPerSession = __gui->displayIntPrompt( "Max ml per session:", "ml",
                                                                                                        __channelStorage[selectedMenuItem].waterStatsStorage.maxMlPerSession, 
                                                                                                        0, 1000, 20);

        __curChannel = selectedMenuItem;        
        writeCurChannelStorage();
       
       setNextState(PRGM_STATE_INITIALIZING);
        
    }
  
}

 





