#include <Gui.h>

void doMainMenuAction(byte selectedMenuItem);

GUI *                   __gui;

static const char GEN_MENU_ITEMS_CHAN_1[] PROGMEM = "Menu 1";
static const char GEN_MENU_ITEMS_CHAN_2[] PROGMEM = "Menu 2";
static const char GEN_MENU_ITEMS_CHAN_3[] PROGMEM = "Menu 3";
static const char GEN_MENU_ITEMS_CHAN_4[] PROGMEM = "Menu 4";
static const char GEN_MENU_ITEMS_CHAN_5[] PROGMEM = "Menu 5";

static const char* const MENU_ITEMS[] PROGMEM = {GEN_MENU_ITEMS_CHAN_1, GEN_MENU_ITEMS_CHAN_2, GEN_MENU_ITEMS_CHAN_3, GEN_MENU_ITEMS_CHAN_4, GEN_MENU_ITEMS_CHAN_5};

static const char MENU_TITLE[] PROGMEM = "TEST GUI";

static const Menu_t MAIN_MENU = {
  MENU_TITLE,
  MENU_ITEMS,
  5
};



/** Affiche le choix de l'utilisateur */
void doMainMenuAction(byte selectedMenuItem)
{ 
  String msg = F("Menu selected: ");
  msg += selectedMenuItem;
  __gui->displayText(msg, F("TEST GUI"), true);
    
}

void setup() {
  __gui = new GUI(0x3F, 5);  
}

void loop() {
   doMainMenuAction( __gui->displayMenu(MAIN_MENU) );
   
  char text[7][LCD_COLUMNS_+1];

  strcpy_P(text[0], PSTR("Ceci est un texte"));
  strcpy_P(text[1], PSTR("Trop bien !!"));
  strcpy_P(text[2], PSTR("Pas mal ..."));
  strcpy_P(text[3], PSTR("Pas glop :("));
  strcpy_P(text[4], PSTR("CON ..."));
  strcpy_P(text[5], PSTR("Mignon @-@"));
  strcpy_P(text[6], PSTR("DEBILE !"));
  
  __gui->displayText2((char**)text,7, F("TEST GUI"));
  __gui->displayIntPrompt( F("Year:"), F(""), 2018, 2018, 2050, 1);
  __gui->displayText(F("Done!"), NULL);

}
