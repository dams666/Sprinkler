#include "GUI.h"

void doMainMenuAction(byte selectedMenuItem);

GUI *                   __gui;

/* Menu principal */
static const char* MAIN_MENU_ITEMS[] = {
  "Menu 1",
  "Menu 2",
  "Menu 3",
  "Menu 4",
  "Menu 5",
  "Menu 6"
};
static const Menu_t MAIN_MENU = {
  "TEST GUI",
  MAIN_MENU_ITEMS,
  6,
  &doMainMenuAction
};



/** Affiche le choix de l'utilisateur */
void doMainMenuAction(byte selectedMenuItem)
{ 
  String msg = "Menu selected: ";
  msg += String(selectedMenuItem);
  
  __gui->displayText(msg, "TEST GUI", true);
  __gui->displayMenu(MAIN_MENU);
  
}

void setup() {
  // put your setup code here, to run once:
__gui = new GUI(0x3F, 5);
__gui->displayMenu(MAIN_MENU);
}

void loop() {
  // put your main code here, to run repeatedly:

}
