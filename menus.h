#ifndef __MENUS_H__
#define __MENUS_H__

#include "GUI.h"

// -------------------------------------------------------
// MENU PRINCIPAL
// -------------------------------------------------------

void doMainMenuAction(byte);

/* Menu principal */
static const char* MAIN_MENU_ITEMS[] = {
  "Start !",
  "Get statistics",
  "Configure"
};
static const Menu_t MAIN_MENU = {
  "SPRINKLER",
  MAIN_MENU_ITEMS,
  3,
  &doMainMenuAction
};


// -------------------------------------------------------
// SOUS MENU : STATISTICS 
// -------------------------------------------------------

void doStatisticsMenuAction(byte);
 
/* Sous menu pour Dr Freeman */
static const char* STATISTICS_MENU_ITEMS[] = {
  "Channel 1",
  "Channel 2",
  "Channel 3",
  "Channel 4",
  "Channel 5",
  "Channel 6",
  "Return to main"
};

static const Menu_t STATISTICS_MENU = {
  "Statistics",
  STATISTICS_MENU_ITEMS,
  7,
  &doStatisticsMenuAction
};

// -------------------------------------------------------
// SOUS MENU : CONFIGURATION
// -------------------------------------------------------

void doConfigureMenuAction(byte);
 
/* Sous menu pour Dr Freeman */
static const char* CONFIGURE_MENU_ITEMS[] = {
  "Channel 1",
  "Channel 2",
  "Channel 3",
  "Channel 4",
  "Channel 5",
  "Channel 6",
  "Factory settings",
  "Return to main"
};
static const Menu_t CONFIGURE_MENU = {
  "Configuration",
  CONFIGURE_MENU_ITEMS,
  8,
  &doConfigureMenuAction
};
 

#endif
