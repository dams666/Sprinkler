#ifndef __MENUS_H__
#define __MENUS_H__

#include "GUI.h"
#include <avr/pgmspace.h>

// -------------------------------------------------------
// MENU PRINCIPAL
// -------------------------------------------------------

void doMainMenuAction(byte);

static const char MAIN_MENU_ITEMS_1[] PROGMEM = "Start !";
static const char MAIN_MENU_ITEMS_2[] PROGMEM = "Get statistics";
static const char MAIN_MENU_ITEMS_3[] PROGMEM = "Configure";
  
/* Menu principal */
static const char* const MAIN_MENU_ITEMS[] PROGMEM = {MAIN_MENU_ITEMS_1, MAIN_MENU_ITEMS_2, MAIN_MENU_ITEMS_3};

static const Menu_t MAIN_MENU = {
  "SPRINKLER",
  MAIN_MENU_ITEMS,
  3,
  &doMainMenuAction
};

static const char STATISTICS_MENU_ITEMS_CHAN_1[] PROGMEM = "Channel 1";
static const char STATISTICS_MENU_ITEMS_CHAN_2[] PROGMEM = "Channel 2";
static const char STATISTICS_MENU_ITEMS_CHAN_3[] PROGMEM = "Channel 3";
static const char STATISTICS_MENU_ITEMS_CHAN_4[] PROGMEM = "Channel 4";
static const char STATISTICS_MENU_ITEMS_CHAN_5[] PROGMEM = "Channel 5";
static const char STATISTICS_MENU_ITEMS_CHAN_6[] PROGMEM = "Channel 6";

static const char* const STATISTICS_MENU_ITEMS_CHANN[] PROGMEM = {STATISTICS_MENU_ITEMS_CHAN_1, STATISTICS_MENU_ITEMS_CHAN_2, STATISTICS_MENU_ITEMS_CHAN_3, STATISTICS_MENU_ITEMS_CHAN_4, STATISTICS_MENU_ITEMS_CHAN_5, STATISTICS_MENU_ITEMS_CHAN_6};

// -------------------------------------------------------
// SOUS MENU : STATISTICS 
// -------------------------------------------------------

void doStatisticsMenuAction(byte);

static const Menu_t STATISTICS_MENU = {
  "Statistics",
  STATISTICS_MENU_ITEMS_CHANN,
  7,
  &doStatisticsMenuAction
};

// -------------------------------------------------------
// SOUS MENU : CONFIGURATION
// -------------------------------------------------------

void doConfigureMenuAction(byte);
 
static const Menu_t CONFIGURE_MENU = {
  "Configuration",
  STATISTICS_MENU_ITEMS_CHANN,
  8,
  &doConfigureMenuAction
};
 

#endif
