#ifndef __MENUS_H__
#define __MENUS_H__

#include "GUI.h"

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
  3,
  &doMainMenuAction
};

// -------------------------------------------------------
// SOUS MENU : STATISTICS 
// -------------------------------------------------------

static const char STATISTICS_MENU_TITLE[] PROGMEM = "Statistics";

static const char* const STATISTICS_MENU_ITEMS[] PROGMEM = {GEN_MENU_ITEMS_CHAN_1, GEN_MENU_ITEMS_CHAN_2, GEN_MENU_ITEMS_CHAN_3, GEN_MENU_ITEMS_CHAN_4, GEN_MENU_ITEMS_CHAN_5, GEN_MENU_ITEMS_CHAN_6, GEN_MENU_ITEMS_RETURN};

void doStatisticsMenuAction(byte);

static const Menu_t STATISTICS_MENU = {
  STATISTICS_MENU_TITLE,
  STATISTICS_MENU_ITEMS,
  7,
  &doStatisticsMenuAction
};

// -------------------------------------------------------
// SOUS MENU : CONFIGURATION
// -------------------------------------------------------

static const char CONFIG_MENU_TITLE[] PROGMEM = "Configuration";

static const char CONFIG_MENU_ITEMS_DATETIME[] PROGMEM = "Date / Time";
static const char CONFIG_MENU_ITEMS_FACT[] PROGMEM = "Factory settings";

static const char* const CONFIG_MENU_ITEMS[] PROGMEM = {GEN_MENU_ITEMS_CHAN_1, GEN_MENU_ITEMS_CHAN_2, GEN_MENU_ITEMS_CHAN_3, GEN_MENU_ITEMS_CHAN_4, GEN_MENU_ITEMS_CHAN_5, GEN_MENU_ITEMS_CHAN_6, CONFIG_MENU_ITEMS_DATETIME, CONFIG_MENU_ITEMS_FACT, GEN_MENU_ITEMS_RETURN};

void doConfigureMenuAction(byte);
 
static const Menu_t CONFIGURE_MENU = {
  CONFIG_MENU_TITLE,
  CONFIG_MENU_ITEMS,
  9,
  &doConfigureMenuAction
};
 

#endif
