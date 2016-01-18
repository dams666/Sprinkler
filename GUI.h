#ifndef __GUI_H__
#define __GUI_H__

class GUI;
class LiquidCrystal_I2C;

extern GUI * __gui;

#include <Arduino.h>

#define LCD_ROWS_         4
#define LCD_COLUMNS_      20

#define btn_up_pin        11
#define btn_dwn_pin       10
#define btn_cfm_pin       12
#define btn_cnc_pin       13

#define btn_up            0
#define btn_dwn           1
#define btn_cfm           2
#define btn_cnc           3

#define BTN_NB            4

#define LCD_CLEAR()\
__gui->lcd->clear();

#define LCD_PRINT(col, line, msg)\
__gui->lcd->setCursor(col,line);\
__gui->lcd->print(msg);\
delay(50);

typedef struct {
  byte pin;
  volatile uint16_t interruptCount;
  void (*interruptFnct)(); 
} BtnInterf_t;

/* Structure d'un menu */
typedef struct {
  const char* prompt;     // Titre du menu
  const char** items;     // Tableau de choix du menu
  const uint8_t nbItems;  // Nombre de choix possibles
  void (*callbackFnct)(uint8_t menuItemSelected); // Pointeur sur fonction pour gérer le choix de l'utilisateur
} Menu_t;
 
/* Listes des touches de la shield lcd DFrobots */
typedef enum {
  BP_NONE,   // Aucun bouton appuyé
  BP_SELECT, // Bouton SELECT
  BP_CANCEL, // Bouton cancel
  BP_LEFT,   // Bouton gauche
  BP_RIGTH,  // Bouton droite
  BP_UP,     // Bouton haut
  BP_DOWN    // Bouton bas
} Button_t;

class GUI
{
  public:
  
  bool                sleepingMode;
  unsigned long       lasTimeBtnCall;
  BtnInterf_t *       buttonInterface;  
  
  LiquidCrystal_I2C * lcd;

  GUI();

  void displayMenu(const Menu_t &menu);
  
  void displayChoice(byte selectedMenuItem, const char** items);

  Button_t readPushButton(void);

  bool isButtonPressed();

  protected:
  
  void centerText(const char * src);
  void scrollBarVert(byte percent, byte column, byte row, byte v_height);
};




#endif /* _MENU_H_ */
