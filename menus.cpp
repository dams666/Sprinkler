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
          LCD_CLEAR();
          LCD_PRINT(0,1, "All channels are OFF"); 
          delay(2000);
        } else {

          __MOD_moistureSensors->reset();
          __MOD_waterStats->reset();
          __MOD_valves->reset();
          
          __programState = PRGM_STATE_ACTIVATING_MOISTURE_SENSORS;
        }
        
        break;
      case 2:
        __gui->displayMenu(CONFIGURE_MENU); 
        break;    
  
    }
  
}


/** Affiche le choix de l'utilisateur */
void doConfigureMenuAction(byte selectedMenuItem)
{ 
     switch (selectedMenuItem) // See which menu item is selected and execute that correspond function
    {
      case 6:
        // restore defaults    
        break;
      case 7:
        __gui->displayMenu(MAIN_MENU);    
        break;
      
      default:
        
        //----------------------------------------------------------------
        // Activation de la vanne
      
        __channelConfig[selectedMenuItem].active = __gui->displayYNPrompt("ACTIVATE ?", __channelConfig[selectedMenuItem].active); 
          
        LCD_CLEAR();
      
        char str[80];
      
        if (__channelConfig[selectedMenuItem].active)
        {
          sprintf(str, "Channel %i is ON", selectedMenuItem +1);
        } else {
          sprintf(str, "Channel %i is OFF", selectedMenuItem +1);
        }
      
        __gui->displayText(str);
        
        //----------------------------------------------------------------
        // définition du max ml par session
        __gui->displayIntPrompt("Max ml per session:", "ml", 400, 0, 1000, 20);
        
       __programState = PRGM_STATE_INITIALIZING;
        
    }
  
}

 





