#ifndef __MENUS_H__
#define __MENUS_H__

#include "GUI.h"

/* Listes des touches de la shield lcd DFrobots */
typedef enum {
  L_MENU_MAIN       = 0,
  L_MENU_STATISTICS = 1,
  L_MENU_CONFIGURE  = 2,
} Menu_l;

static const char GEN_MENU_ITEMS_CHAN_1[] PROGMEM = "Channel 1";
static const char GEN_MENU_ITEMS_CHAN_2[] PROGMEM = "Channel 2";
static const char GEN_MENU_ITEMS_CHAN_3[] PROGMEM = "Channel 3";
static const char GEN_MENU_ITEMS_CHAN_4[] PROGMEM = "Channel 4";
static const char GEN_MENU_ITEMS_CHAN_5[] PROGMEM = "Channel 5";
static const char GEN_MENU_ITEMS_CHAN_6[] PROGMEM = "Channel 6";
static const char GEN_MENU_ITEMS_RETURN[] PROGMEM = "Return to main";

// -------------------------------------------------------
// MENU PRINCIPAL
// -------------------------------------------------------

void doMainMenuAction(byte);

static const char MAIN_MENU_TITLE[] PROGMEM = "SPRINKLER";

static const char MAIN_MENU_ITEMS_1[] PROGMEM = "Start !";
static const char MAIN_MENU_ITEMS_2[] PROGMEM = "Get statistics";
static const char MAIN_MENU_ITEMS_3[] PROGMEM = "Configure";
  
/* Menu principal */
static const char* const MAIN_MENU_ITEMS[] PROGMEM = {MAIN_MENU_ITEMS_1, MAIN_MENU_ITEMS_2, MAIN_MENU_ITEMS_3};

static const Menu_t MAIN_MENU = {
  MAIN_MENU_TITLE,
  MAIN_MENU_ITEMS,
  3
};

// -------------------------------------------------------
// SOUS MENU : STATISTICS 
// -------------------------------------------------------

static const char STATISTICS_MENU_TITLE[] PROGMEM = "Statistics";

#if MAX_CHANNELS_ == 3
static const char* const STATISTICS_MENU_ITEMS[] PROGMEM = {GEN_MENU_ITEMS_CHAN_1, GEN_MENU_ITEMS_CHAN_2, GEN_MENU_ITEMS_CHAN_3, GEN_MENU_ITEMS_RETURN};
#elif MAX_CHANNELS_ == 4
static const char* const STATISTICS_MENU_ITEMS[] PROGMEM = {GEN_MENU_ITEMS_CHAN_1, GEN_MENU_ITEMS_CHAN_2, GEN_MENU_ITEMS_CHAN_3, GEN_MENU_ITEMS_CHAN_4, GEN_MENU_ITEMS_RETURN};
#elif MAX_CHANNELS_ == 5
static const char* const STATISTICS_MENU_ITEMS[] PROGMEM = {GEN_MENU_ITEMS_CHAN_1, GEN_MENU_ITEMS_CHAN_2, GEN_MENU_ITEMS_CHAN_3, GEN_MENU_ITEMS_CHAN_4, GEN_MENU_ITEMS_CHAN_5, GEN_MENU_ITEMS_RETURN};
#else
static const char* const STATISTICS_MENU_ITEMS[] PROGMEM = {GEN_MENU_ITEMS_CHAN_1, GEN_MENU_ITEMS_CHAN_2, GEN_MENU_ITEMS_CHAN_3, GEN_MENU_ITEMS_CHAN_4, GEN_MENU_ITEMS_CHAN_5, GEN_MENU_ITEMS_CHAN_6, GEN_MENU_ITEMS_RETURN};
#endif

void doStatisticsMenuAction(byte);

static const Menu_t STATISTICS_MENU = {
  STATISTICS_MENU_TITLE,
  STATISTICS_MENU_ITEMS,
  MAX_CHANNELS_ + 1
};

// -------------------------------------------------------
// SOUS MENU : CONFIGURATION
// -------------------------------------------------------

static const char CONFIG_MENU_TITLE[] PROGMEM = "Configuration";

static const char CONFIG_MENU_ITEMS_DATETIME[] PROGMEM = "Date / Time";
static const char CONFIG_MENU_ITEMS_FACT[] PROGMEM = "Factory settings";

#if MAX_CHANNELS_ == 3
static const char* const CONFIG_MENU_ITEMS[] PROGMEM = {GEN_MENU_ITEMS_CHAN_1, GEN_MENU_ITEMS_CHAN_2, GEN_MENU_ITEMS_CHAN_3, CONFIG_MENU_ITEMS_DATETIME, CONFIG_MENU_ITEMS_FACT, GEN_MENU_ITEMS_RETURN};
#elif MAX_CHANNELS_ == 4
static const char* const CONFIG_MENU_ITEMS[] PROGMEM = {GEN_MENU_ITEMS_CHAN_1, GEN_MENU_ITEMS_CHAN_2, GEN_MENU_ITEMS_CHAN_3, GEN_MENU_ITEMS_CHAN_4, CONFIG_MENU_ITEMS_DATETIME, CONFIG_MENU_ITEMS_FACT, GEN_MENU_ITEMS_RETURN};
#elif MAX_CHANNELS_ == 5
static const char* const CONFIG_MENU_ITEMS[] PROGMEM = {GEN_MENU_ITEMS_CHAN_1, GEN_MENU_ITEMS_CHAN_2, GEN_MENU_ITEMS_CHAN_3, GEN_MENU_ITEMS_CHAN_4, GEN_MENU_ITEMS_CHAN_5 CONFIG_MENU_ITEMS_DATETIME, CONFIG_MENU_ITEMS_FACT, GEN_MENU_ITEMS_RETURN};
#else
static const char* const CONFIG_MENU_ITEMS[] PROGMEM = {GEN_MENU_ITEMS_CHAN_1, GEN_MENU_ITEMS_CHAN_2, GEN_MENU_ITEMS_CHAN_3, GEN_MENU_ITEMS_CHAN_4, GEN_MENU_ITEMS_CHAN_5c, GEN_MENU_ITEMS_CHAN_6, CONFIG_MENU_ITEMS_DATETIME, CONFIG_MENU_ITEMS_FACT, GEN_MENU_ITEMS_RETURN};
#endif


void configureDateTime();
void doConfigureMenuAction(byte);
 
static const Menu_t CONFIGURE_MENU = {
  CONFIG_MENU_TITLE,
  CONFIG_MENU_ITEMS,
  MAX_CHANNELS_ + 3
};
 

#endif
