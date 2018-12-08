#ifndef __GUI_H__
#define __GUI_H__

class GUI;
class LiquidCrystal_I2C;

class IRrecv;
class decode_results;

extern GUI * __gui;

#include <Arduino.h>

#define LCD_ROWS_             4
#define LCD_COLUMNS_          20
#ifndef LCD_SLEEPING_TIMEOUT
#define LCD_SLEEPING_TIMEOUT  60000
#endif

#define btn_up_pin            11
#define btn_dwn_pin           10
#define btn_cfm_pin           12
#define btn_cnc_pin           13

#define btn_up                0
#define btn_dwn               1
#define btn_cfm               2
#define btn_cnc               3

#define BTN_NB                4

#define LCD_CLEAR()\
__gui->lcd->clear();

#define LCD_PRINT(col, line, msg)\
__gui->lcd->setCursor(col,line);\
__gui->lcd->print(msg);\
delay(50);


/* Structure d'un menu */
typedef struct {
  const char* prompt;     // Titre du menu
  const char** items;     // Tableau de choix du menu
  const uint8_t nbItems;  // Nombre de choix possibles
  void (*callbackFnct)(uint8_t menuItemSelected); // Pointeur sur fonction pour gérer le choix de l'utilisateur
} Menu_t;
 
/* Listes des touches de la shield lcd DFrobots */
typedef enum {
  BP_NONE     = 0,   // Aucun bouton appuyé
  BP_OK       = 1, // Bouton SELECT
  BP_LEFT     = 2,   // Bouton gauche
  BP_RIGHT    = 3,  // Bouton droite
  BP_UP       = 4,     // Bouton haut
  BP_DOWN     = 5,    // Bouton bas
  BP_ZERO     = 6,
  BP_ONE      = 7,
  BP_TWO      = 8,
  BP_THREE    = 9,
  BP_FOUR     = 10,
  BP_FIVE     = 11,
  BP_SIX      = 12,
  BP_SEVEN    = 13,
  BP_EIGHT    = 14,
  BP_NINE     = 15,
  BP_STAR     = 16,
  BP_HASH     = 17
} Button_t;

class GUI
{
  public:
  
  bool                sleepingMode;
  unsigned long       lasTimeBtnCall;
    
  LiquidCrystal_I2C * lcd;

  IRrecv *        irDetect;
  decode_results *    irIn;
  
  public:
  
  GUI(uint8_t lcd_i2c_addr, uint8_t rc_pin);
  
  int displayIntPrompt(const __FlashStringHelper* msg, const __FlashStringHelper* unit, int start, int min, int max, int step);

  bool displayYNPrompt(const __FlashStringHelper* msg, bool dftYes = false);
  
  void displayMenu(const Menu_t &menu);
  
  void displayText(String msg, const __FlashStringHelper* title = NULL, bool waitKey = true);
  void displayText(const __FlashStringHelper* msg, const __FlashStringHelper* title = NULL, bool waitKey = true);
  void displayText2(const char text[20][LCD_COLUMNS_+1], int len, const __FlashStringHelper* title = NULL);
  
  Button_t readPushButton(void);
  
  void centerText(const __FlashStringHelper * src);
  
  protected:
  
  void scrollBarVert(byte percent, byte column, byte row, byte v_height);
};




#endif /* _MENU_H_ */

