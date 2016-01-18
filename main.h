#ifndef __MAIN_H__
#define __MAIN_H__

#include <Arduino.h>
#include <Wire.h>  // Comes with Arduino IDE
// Get the LCD I2C Library here: 
// https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads
// Move any other LCD libraries to another folder or delete them
// See Library "Docs" folder for possible commands etc.
#include <LiquidCrystal_I2C.h>
#include <phi_interfaces.h>
#include <phi_prompt.h>
#include "Eepromutil.h"

#include "GUI.h"

//#define WITH_SERIAL
#undef WITH_SERIAL

#define RELAY_OFF HIGH
#define RELAY_ON LOW

#define SLEEPING_DURATION_        60000
#define MAX_CHANNELS_             6
  
#ifdef WITH_SERIAL
#define DEBUG_PRINT(msg)\
Serial.print((msg));                              
#else
#define DEBUG_PRINT(msg) 
#endif

#ifdef WITH_SERIAL
#define DEBUG_PRINTLN(msg)\
Serial.println(msg);      \
delay(50);                      
#else
#define DEBUG_PRINTLN(msg) 
#endif
  
extern String* __msg1;
extern String* __msg2;

class MOD_moistureSensors_;
class MOD_waterStats_;
class MOD_valves_;

extern MOD_moistureSensors_  * __MOD_moistureSensors;
extern MOD_waterStats_       * __MOD_waterStats;
extern MOD_valves_           * __MOD_valves;

typedef struct 
{
  bool active = false;
  int maxMlPerSession = 1000;
} chanConf;

extern chanConf __channelConfig[MAX_CHANNELS_];

extern int __curChannel; // identifiant courant de la sortie à inspecter
  
//------------------------------------------------------------------------------------
// PROGRAM STATE
//------------------------------------------------------------------------------------

 enum  programState {
    PRGM_STATE_INITIALIZING,
    PRGM_STATE_CONFIGURE,
    PRGM_STATE_ACTIVATING_MOISTURE_SENSORS,
    PRGM_STATE_READING_MOISTURE_SENSORS,
    PRGM_STATE_INSPECTING_FOR_CHANGES,
    PRGM_STATE_CLOSING_MAIN_VALVE,
    PRGM_STATE_ALERT
  };
  
  extern int __programState;

  /*
  On définit un état spécifique des lors qu'il dure un certain temps
   
   PRGM_STATE_INITIALIZING : 
   
   après allumage ou reboot de l'arduino après plantage. Les vannes secondaires sont ouvertes puis refermées pour dissiper l'eau 
   qui se serait éventuellement accumulée dans les tuyaux entre la vanne princuppale et la vanne secondaire
   
   ETAT(S) PRECEDENT(S) :
   AUCUN
   ETAT(S) SUIVANT(S)   : 
   PRGM_STATE_ACTIVATING_MOISTURE_SENSORS 
   
   PRGM_STATE_ACTIVATING_MOISTURE_SENSORS :
   
   activation de l'ensemble des détecteurs d'humidité. On laisse un court instant passer avant de lire les résultats (cf état PRGM_STATE_READING_MOISTURE_SENSORS)
   
   ETAT(S) PRECEDENT(S) :
   PRGM_STATE_INITIALIZING
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

  int getNbChannelsActivated();
#endif

