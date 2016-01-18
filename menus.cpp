#include "main.h"
#include "menus.h"

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
          __programState = PRGM_STATE_ACTIVATING_MOISTURE_SENSORS;
        }
        break;
      case 2:
        __gui->displayMenu(CONFIGURE_MENU); 
        break;     
      default:
        __curChannel = selectedMenuItem - 1;
        __programState = PRGM_STATE_CONFIGURE;
  
    }
  
}


/** Affiche le choix de l'utilisateur */
void doConfigureMenuAction(byte selectedMenuItem)
{ 
     switch (selectedMenuItem) // See which menu item is selected and execute that correspond function
    {
      case 7:
        __gui->displayMenu(MAIN_MENU);    
        break;
      
      default:
        __curChannel = selectedMenuItem - 1;
        __programState = PRGM_STATE_CONFIGURE;
  
    }
  
}

 





