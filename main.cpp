#include "main.h"
#include "MOD_moisture_sensors.h"
#include "MOD_water_stats.h"
#include "MOD_valves.h"

#define global_style 109
// This is the style of the menu

#define btn_u 34
#define btn_d 32
#define btn_l 30
#define btn_r 28
#define btn_b 26
#define btn_a 24
#define total_buttons 6

// Menu texts
PROGMEM const char top_menu_item00[]="Start !";
PROGMEM const char top_menu_item01[]="Configure valve 1";
PROGMEM const char top_menu_item02[]="Configure valve 2";
PROGMEM const char top_menu_item03[]="Configure valve 3";
PROGMEM const char top_menu_item04[]="Configure valve 4";
PROGMEM const char top_menu_item05[]="Configure valve 5";
PROGMEM const char top_menu_item06[]="Configure valve 6";
PROGMEM const char * const top_menu_items[] = {top_menu_item00, top_menu_item01, top_menu_item02, top_menu_item03, top_menu_item04, top_menu_item05, top_menu_item06};

PROGMEM const char menu_config00[]="Activation";
PROGMEM const char * const menu_configs[] = {menu_config00};


    char mapping[]={1,2,3,4,5,6}; // This is a list of names for each button.
    byte pins[]={btn_u,btn_d,btn_l,btn_r,btn_b,btn_a}; // The digital pins connected to the 6 buttons.
    phi_button_groups my_btns(mapping, pins, total_buttons);
    phi_serial_keypads debug_keypad(&Serial,115200);
    multiple_button_input * keypads[]={&my_btns,&debug_keypad,0};
    char up_keys[]={1,0}; ///< All keys that act as the up key are listed here.
    char down_keys[]={2,0}; ///< All keys that act as the down key are listed here.
    char left_keys[]={3,0}; ///< All keys that act as the left key are listed here.
    char right_keys[]={4,0}; ///< All keys that act as the right key are listed here.
    char enter_keys[]={5,0}; ///< All keys that act as the enter key are listed here.
    char escape_keys[]={6,0}; ///< All keys that act as the escape key are listed here.
    char * function_keys[]={up_keys,down_keys,left_keys,right_keys,enter_keys,escape_keys}; ///< All function key names are gathered here fhr phi_prompt.



// -------------------------------------------------------------------------------------------------

int __programState;

int __curChannel;

String *__msg1;
String *__msg2;

bool __channelActivated[MAX_CHANNELS_];

LiquidCrystal_I2C * __LCD;

MOD_moistureSensors_ *  __MOD_moistureSensors;
MOD_waterStats_ *       __MOD_waterStats;
MOD_valves_ *           __MOD_valves;

// -------------------------------------------------------------------------------------------------

  int getNbChannelsActivated()
  {
    int res = 0;
    for (int thisPin = 0; thisPin < MAX_CHANNELS_; thisPin++)
      res +=__channelActivated[thisPin];

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
    memset(__channelActivated, 0, MAX_CHANNELS_);
    
    __programState = PRGM_STATE_INITIALIZING;
        
    __curChannel = 0;

    __msg1 = new String();
    __msg2 = new String();

    //------------------------------------------------------------------------------------ 
    // INIT LCD
    //------------------------------------------------------------------------------------
    
    __LCD = new LiquidCrystal_I2C (0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address
    __LCD->begin(LCD_COLUMNS_, LCD_ROWS_);         // initialize the __LCD for 20 chars 4 lines

    //------------------------------------------------------------------------------------ 
    // INIT MODULES
    //------------------------------------------------------------------------------------
    
    __MOD_moistureSensors = new MOD_moistureSensors_();
    __MOD_waterStats      = new MOD_waterStats_();
    __MOD_valves          = new MOD_valves_();
     
    //------------------------------------------------------------------------------------ 
    // INIT PHI_PROMPT LIBRARY
    //------------------------------------------------------------------------------------
    

    init_phi_prompt(__LCD, keypads, function_keys, LCD_COLUMNS_, LCD_ROWS_, '~');

    LCD_PRINT(2,0,"=== SPRINKLER ===");

    __MOD_valves->purgeTransitionalCircuit();
    
    DEBUG_PRINTLN("INITIALIZATION : Done");

    delay(3000);

  }

  void alertAction()
  {
    __MOD_valves->closeAllValves();
        
    DEBUG_PRINT("ALERT : ");
    DEBUG_PRINTLN(*__msg1);
    
    while(true)
    {
      __LCD->clear();
      delay(100);    
      LCD_PRINT(3,0, "=== ALERT ! ===");

      LCD_PRINT(0,2, *__msg1);
      if ((*__msg2) != "") LCD_PRINT(0,3, *__msg2);
      
      delay(1000);
    }
  }


void initializeAction()
{
  int menu_pointer_1=0; // This stores the menu choice the user made.
  phi_prompt_struct myMenu; // This structure stores the main menu.

// Initialize the top menu
  myMenu.ptr.list=(char**)&top_menu_items; // Assign the list to the pointer
  myMenu.low.i=0; // Default item highlighted on the list
  myMenu.high.i=4; // Last item of the list is size of the list - 1.
  myMenu.width=LCD_COLUMNS_-((global_style&phi_prompt_arrow_dot)!=0)-((global_style&phi_prompt_scroll_bar)!=0); // Auto fit the size of the list to the screen. Length in characters of the longest list item.
  myMenu.step.c_arr[0]=LCD_ROWS_-1; // rows to auto fit entire screen
  myMenu.step.c_arr[1]=1; // one col list
  myMenu.step.c_arr[2]=0; // y for additional feature such as an index
  myMenu.step.c_arr[3]=LCD_COLUMNS_-4-((global_style&phi_prompt_index_list)!=0); // x for additional feature such as an index
  myMenu.col=0; // Display menu at column 0
  myMenu.row=1; // Display menu at row 1
  myMenu.option=global_style; // Option 0, display classic list, option 1, display 2X2 list, option 2, display list with index, option 3, display list with index2.
  
  //while(1) // This loops every time a menu item is selected.
  {
    __LCD->clear();  // Refresh menu if a button has been pushed
    center_text("SPRINKLER");

    myMenu.option=global_style; // This makes sure the change of menu style is updated if there is a menu style option in this level of menu.
    myMenu.width=LCD_COLUMNS_-((global_style&phi_prompt_arrow_dot)!=0)-((global_style&phi_prompt_scroll_bar)!=0); // This makes sure the change of menu style is updated if there is a menu style option in this level of menu. Auto fit the size of the list to the screen. Length in characters of the longest list item.
    myMenu.step.c_arr[3]=LCD_COLUMNS_-4-((global_style&phi_prompt_index_list)!=0); // This makes sure the change of menu style is updated if there is a menu style option in this level of menu. x for additional feature such as an index
    select_list(&myMenu); // Use the select_list to ask the user to select an item of the list, that is a menu item from your menu.
    menu_pointer_1=myMenu.low.i; // Get the selected item number and store it in the menu pointer.
    
    switch (menu_pointer_1) // See which menu item is selected and execute that correspond function
    {
      case 0:
        __programState = PRGM_STATE_ACTIVATING_MOISTURE_SENSORS;
        break;
      
      default:
        __curChannel = menu_pointer_1 - 1;
        __programState = PRGM_STATE_CONFIGURE;
  
    }
  }
}

/*

// Menu functions go here. The functions are called when their menu items are selected. They are called only once so if you want to do something repeatedly, make sure you have a while loop.
void top_menu_function_1() //Replace this with the actual function of menu item #1
{
  __LCD->clear();
  __LCD->print("Collecting data.");
  __LCD->setCursor(0,1);
  __LCD->print("-Your code here-");
  wait_on_escape(4000);
}

void top_menu_function_2() //Replace this with the actual function of menu item #2
{
  __LCD->clear();
  __LCD->print("Sending data....");
  __LCD->setCursor(0,1);
  __LCD->print("-Your code here-");
  wait_on_escape(4000);
}

*/

void configureAction()
{
  char buffer[32];
  byte opt=global_style, yn;
  
  __LCD->clear();
  strcpy_P(buffer,(char*)pgm_read_word(menu_configs));
  yn=yn_dialog(buffer);
  
  __channelActivated [__curChannel] = yn;

  __LCD->clear();
  __LCD->print("Setting stored !");
  wait_on_escape(2000);

 __programState = PRGM_STATE_INITIALIZING;
}


  void inspectForChangesAction()
  {      
    // détection de changement d'état du moisture sensor
    if (__channelActivated[__curChannel] && __MOD_moistureSensors->state[__curChannel] != __MOD_moistureSensors->prevState[__curChannel]) 
    {
      __MOD_valves->changeValveState();
    }
    // détection de reprise suite à l'extinction d'une autre vanne
    if (__channelActivated[__curChannel] && __MOD_valves->state[__curChannel] != __MOD_moistureSensors->state[__curChannel]) 
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

      LCD_PRINT(0,3, "SLEEPING...      ");
      
      DEBUG_PRINTLN("SLEEPING FOR A FEW MINUTES");
      
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
          
        initializeAction();
        
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
  

