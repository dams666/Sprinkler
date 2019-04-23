#ifndef __MAIN_H__
#define __MAIN_H__

#include <Arduino.h>

#include <Wire.h>  // Comes with Arduino IDE
// Get the LCD I2C Library here: 
// https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads
// Move any other LCD libraries to another folder or delete them
// See Library "Docs" folder for possible commands etc.
#include <Eepromutil.h>
#include <Gui.h>
#include <ModuleConfig.h>
  
#ifdef WITH_SERIAL
#define DEBUG_PRINT(msg)\
Serial.print((msg));                              
#else
#define DEBUG_PRINT(msg) 
#endif

#ifdef WITH_SERIAL
#define DEBUG_PRINTLN(msg)\
Serial.println((msg));      \
delay(50);                      
#else
#define DEBUG_PRINTLN(msg) 
#endif


#define BUZZER_PIN LED_BUILTIN
#define SLEEPING_DURATION_        600000  // 1h
#define WITH_DS1307
//#undef WITH_DS1307

//------------------------------------
// GUI
//------------------------------------

#define LCD_I2C_ADDR 0x3F
#define RC_PIN 5

//-------------------------------------------------------------------------------------------------------------
// VARIABLES DE LA MACHINE A ETAT
//-------------------------------------------------------------------------------------------------------------

 enum  programState {
    PRGM_STATE_UNDEFINED = 0,
    PRGM_STATE_SHOW_MENU = 1,
    PRGM_STATE_SLEEPING = 2,
    PRGM_STATE_ACTIVATING_MOISTURE_SENSORS = 3,
    PRGM_STATE_READING_MOISTURE_SENSORS = 4,
    PRGM_STATE_INSPECTING_FOR_CHANGES = 5,
    PRGM_STATE_ALERT = 6,
    PRGM_STATE_NB = 7
  };

extern volatile uint8_t             __programState;
extern volatile uint8_t             __programNextState;
/* 
 *  ON définit un byte pour éviter les problèmes d'interférence avec l'interrupt
 *  cf: https://www.arduino.cc/reference/en/language/variables/variable-scope--qualifiers/volatile/
*/

extern volatile unsigned long   __programStateMillis;
extern volatile unsigned long   __actionMillis;
extern volatile unsigned long   __nextActionMillis;

typedef void (*action_t)(void);

extern action_t __array_actions[PRGM_STATE_NB]; 
  /*
  On définit un état spécifique des lors qu'il dure un certain temps
   
   PRGM_STATE_SHOW_MENU : 
   
   après allumage ou reboot de l'arduino après plantage. Les vannes secondaires sont ouvertes puis refermées pour dissiper l'eau 
   qui se serait éventuellement accumulée dans les tuyaux entre la vanne princuppale et la vanne secondaire

   on affiche le menu principal
   
   ETAT(S) PRECEDENT(S) :
   AUCUN
   ETAT(S) SUIVANT(S)   : 
   PRGM_STATE_ACTIVATING_MOISTURE_SENSORS 
   
   PRGM_STATE_ACTIVATING_MOISTURE_SENSORS :
   
   activation de l'ensemble des détecteurs d'humidité. On laisse un court instant passer avant de lire les résultats (cf état PRGM_STATE_READING_MOISTURE_SENSORS)
   
   ETAT(S) PRECEDENT(S) :
   PRGM_STATE_SHOW_MENU
   INSPECTING FOR CHANGES : si toutes les vannes secondaires ont été fermées, on passe en veille
   
   ETAT(S) SUIVANT(S)   : 
   PRGM_STATE_READING_MOISTURE_SENSORS
   
   PRGM_STATE_READING_MOISTURE_SENSORS :
   
   lecture des détecteurs d'humidité
   
   ETAT(S) PRECEDENT(S) : 
   PRGM_STATE_ACTIVATING_MOISTURE_SENSORS
   INSPECTING FOR CHANGES : si une vanne secondaire est ouverte, donc une plante est arrosée
   
   ETAT(S) SUIVANT(S)   : 
   PRGM_STATE_INSPECTING_FOR_CHANGES
   
   PRGM_STATE_INSPECTING_FOR_CHANGES :
   
   inspection des changements de valeur des détecteurs d'humidité, et ouverture / fermeture des vannes
   
   ETAT(S) PRECEDENT(S) : 
   INSPECTING FOR CHANGES : si incrémentation de l'identifiant de vanne secondaire à inspecter
   
   ETAT(S) SUIVANT(S)   : 
   INSPECTING FOR CHANGES : si incrémentation de l'identifiant de vanne secondaire à inspecter
   PRGM_STATE_CLOSING_MAIN_VALVE : si une vanne secondaire doit être fermée, on ferme d'abord la vanne principale et on attend que l'eau ne circule plus
   
   PRGM_STATE_CLOSING_MAIN_VALVE :
   
   avant de fermer une vanne secondaire, on ferme la vanne principale et on attend un instant que l'eau ne circule plus (l'effet de fermeture n'est pas toujours immédiat).
   Comme la fermeture n'est pas immédiate, cela permet de maintenir une pression nulle dans les tuyaux entre la vanne principale et les vannes intermédiaires.
   
   ETAT(S) PRECEDENT(S) : 
   INSPECTING FOR CHANGES
   
   ETAT(S) SUIVANT(S)   : 
   
   */
  void sprinklerInit();
  
  void sprinklerAction();
  
  void setProgramAction(uint8_t state, uint8_t delay_ = 100);
  
  bool isAlertState();

/*
static const char PRGM_STATE_NAME_UNDEFINED[] PROGMEM     = "UNDEFINED";
static const char PRGM_STATE_NAME_INITIALIZING[] PROGMEM  = "INITIALIZING";
static const char PRGM_STATE_NAME_SLEEPING[] PROGMEM      = "SLEEPING";
static const char PRGM_STATE_NAME_ACTIVATING_MOISTURE_SENSORS[] PROGMEM = "ACT MOIST SENS";
static const char PRGM_STATE_NAME_INSPECTING_FOR_CHANGES[] PROGMEM = "READ MOIST SENS";
static const char PRGM_STATE_NAME_ALERT[] PROGMEM = "ALERT";


// Menu principal
const char* const PRGM_STATE_NAMES[] = {PRGM_STATE_NAME_UNDEFINED, PRGM_STATE_NAME_INITIALIZING, PRGM_STATE_NAME_SLEEPING, PRGM_STATE_NAME_ACTIVATING_MOISTURE_SENSORS, PRGM_STATE_NAME_INSPECTING_FOR_CHANGES, PRGM_STATE_NAME_ALERT};
*/

//-------------------------------------------------------------------------------------------------------------
// VARIABLES TRANSVERSES UTILISEES PAR LES DIFFERENTS ETATS
//-------------------------------------------------------------------------------------------------------------

extern String                __msg;

// Modules
class MOD_moistureSensors_;
class MOD_waterStats_;
class MOD_valves_;

// GUI
extern MOD_moistureSensors_  * __MOD_moistureSensors;
extern MOD_waterStats_       * __MOD_waterStats;
extern MOD_valves_           * __MOD_valves;

extern GUI * __gui;

#define LCD_CLEAR()\
__gui->lcd->clear();

#define LCD_PRINT(col, line, msg)\
__gui->lcd->setCursor(col,line);\
__gui->lcd->print(msg);\
delay(50);

void globalFlowIncPulseCounter();

typedef struct 
{
  bool active = false;
  uint16_t  maxMlPerSession;
    
} chanConf;

extern chanConf __channelConf[MAX_CHANNELS_];

void readChannelConfs();
void writeChannelConfs();
uint8_t getNbChannelsActivated();

// identifiant courant de la sortie à inspecter
extern uint8_t __curChannel; 

// menu à afficher
extern uint8_t __curMenu;

#endif

