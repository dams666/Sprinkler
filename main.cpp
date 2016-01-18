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
      res +=__channelConfig[thisPin].active;
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
      __gui->lcd->clear();
      delay(100);    
      LCD_PRINT(3,0, "=== ALERT ! ===");

      LCD_PRINT(0,2, *__msg1);
      if ((*__msg2) != "") LCD_PRINT(0,3, *__msg2);
      
      delay(1000);
    }
  }




void configureAction()
{
  char buffer[32];

  //----------------------------------------------------------------
  // Activation de la vanne
  
  byte yn;
  
  __gui->lcd->clear();
  //strcpy_P(buffer,(char*)pgm_read_word(config_menu_items));
  //yn=yn_dialog(buffer);

  //__channelConfig[__curChannel].active = yn;

  /*
  __gui->lcd->clear();
  __gui->lcd->setCursor(0, 1);
  char str[80];

  if (__channelConfig[__curChannel].active)
  {
    sprintf(str, "Channel %i is ON", __curChannel +1);
    __gui->lcd->print(str);
  } else {
    sprintf(str, "Channel %i is OFF", __curChannel +1);
    __gui->lcd->print(str);
  }
  
  wait_on_escape(2000);

  */
  //----------------------------------------------------------------
  // définition du max ml par session
/*  
  int user_input=1000; // This is the storage for the integer 
  phi_prompt_struct myIntegerInput; // This struct stores information for library functions
  myIntegerInput.ptr.i_buffer=&user_input; // Pass the address of user_input to the library. After library function call, user input will be stored in this variable. Note the use of “&”. 
  myIntegerInput.low.i=0; // Lower limit. The number wraps to 20 when decreased from 0. 
  myIntegerInput.high.i=2000; // Upper limit. The number wraps to 0 when increased from 20. 
  myIntegerInput.step.i=50; // Step size. You will get 0, 2, 4, 6, 8, etc if you set it to 2. 
  myIntegerInput.col=7; // Display the number at column 7 
  myIntegerInput.row=1; // Display the number at row 1 
  myIntegerInput.width=2; // The number occupies 2 character space. 
  myIntegerInput.option=0; // Option 0, space pad right, 1, zero pad left, 2, space pad left. 
  __gui->lcd->clear(); // Clear the _LCD-> 
  __gui->lcd->print("Max ml per session:"); // Prompt user for input 
  input_integer(&myIntegerInput); // This calls the library function. The initial number will be displayed first and the functions waits for the user to press up/down to change the number and enter to confirm, after which it stores the new number in user_input. Notice the "&" in front of the myIntegerInput struct.

  __channelConfig[__curChannel].maxMlPerSession = user_input;
  */
  /*
  // Use sscanf to turn text input into number, that is if the input is number
  __gui->lcd->clear();
  __gui->lcd->print("Max ml/session:");
  __gui->lcd->print(user_input);
  wait_on_escape(2000);
  */
  

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

      __gui->lcd->clear();
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
        
      case PRGM_STATE_CONFIGURE:
        DEBUG_PRINTLN("--- CONFIGURATION --- ");
          
        configureAction();
        
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
  

