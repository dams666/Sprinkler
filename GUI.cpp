#include "GUI.h"
#include <LiquidCrystal_I2C.h>
#include <EnableInterrupt.h>

void interruptFunctionUp()
{ 
  __gui-> lasTimeBtnCall = millis(); 
  ++__gui->buttonInterface[btn_up].interruptCount;
}

void interruptFunctionDwn()
{
  __gui-> lasTimeBtnCall = millis();
  ++__gui->buttonInterface[btn_dwn].interruptCount;
}

void interruptFunctionCfm()
{
  __gui-> lasTimeBtnCall = millis();
  ++__gui->buttonInterface[btn_cfm].interruptCount;
  }

void interruptFunctionCnc()
{
  __gui-> lasTimeBtnCall = millis();
  ++__gui->buttonInterface[btn_cnc].interruptCount;
}


GUI::GUI()
{
 
  /* Objet LCD sur les broches utilisées par la shield LCD DFrobots */
  lcd = new LiquidCrystal_I2C(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

  buttonInterface= new BtnInterf_t[BTN_NB];

  buttonInterface[0] = {btn_up_pin,  0, interruptFunctionUp};
  buttonInterface[1] = {btn_dwn_pin, 0, interruptFunctionDwn};
  buttonInterface[2] = {btn_cfm_pin, 0, interruptFunctionCfm};
  buttonInterface[3] = {btn_cnc_pin, 0, interruptFunctionCnc};

  for (int ii = 0; ii< BTN_NB; ++ii)
  {
    pinMode(buttonInterface[ii].pin, INPUT_PULLUP);  // See http://arduino.cc/en/Tutorial/DigitalPins
    enableInterrupt(buttonInterface[ii].pin, buttonInterface[ii].interruptFnct, FALLING);
  }
  sleepingMode = false;
  lasTimeBtnCall = 0;
  /* Configuration du LCD */
  lcd->begin(20, 4);
}


bool GUI::isButtonPressed()
{
  return buttonInterface[btn_up].interruptCount || 
        buttonInterface[btn_dwn].interruptCount || 
        buttonInterface[btn_cnc].interruptCount || 
        buttonInterface[btn_cfm].interruptCount;
}

/** Fonction retournant le bouton appuyé (s’il y en a un). */
Button_t GUI::readPushButton(void)
{ 
  if (sleepingMode)
  {
    if  (isButtonPressed())
    {
      sleepingMode = false;
      lcd->backlight();
    } else {
      delay(500);
      return BP_NONE;
    }
  } else
  {
    if  (!isButtonPressed())
    {
      if (millis() - lasTimeBtnCall > 10000)
      {
        sleepingMode = true;
        lcd->noBacklight();
      }
      return BP_NONE;
    }
  }
  
  /* Test suivant les fourchettes de valeurs */
  if (buttonInterface[btn_up].interruptCount > 0)
  {
    buttonInterface[btn_up].interruptCount = 0;
    return BP_UP; 
  }
  if (buttonInterface[btn_dwn].interruptCount > 0)
  {
    buttonInterface[btn_dwn].interruptCount = 0;
    return BP_DOWN; 
  }
  if (buttonInterface[btn_cfm].interruptCount > 0)
  {
    buttonInterface[btn_cfm].interruptCount = 0;
    return BP_SELECT;
  }
  if (buttonInterface[btn_cnc].interruptCount > 0)
  {
    buttonInterface[btn_cnc].interruptCount = 0;
    return BP_CANCEL;
  }
  
  /* Par défaut aucun bouton n'est appuyé */
  return BP_NONE;
}

/**
 * \details This function displays a short message in SRAM centered with the display size.
 * This outputs a text center-aligned on the display depending on the display size.
 * Eg. center_text("Introduction") outputs ">>>>Introduction<<<<" on 20 column display.
 * \param src This is the string to be displayed.
 */
void GUI::centerText(const char * src)
{
  byte j=0;
  for (byte i=0;i<LCD_COLUMNS_;i++)
  {
    if (i<LCD_COLUMNS_/2-(strlen(src)-strlen(src)/2)) lcd->write('>');
    else if (i>=LCD_COLUMNS_/2+strlen(src)/2) lcd->write('<');
    else 
    {
      lcd->write(src[j]);
      j++;
    }
  }
}



/** Affiche le menu passé en argument */
void GUI::displayMenu(const Menu_t &menu)
{ 
  /* Variable pour le menu */
  byte selectedMenuItem = 0;   // Choix selectionné
  byte shouldExitMenu = false; // Devient true quand l'utilisateur veut quitter le menu
  Button_t buttonPressed;      // Contient le bouton appuyé
 
  /* Tant que l'utilisateur ne veut pas quitter pas le menu */
  while(!shouldExitMenu)
  {
    /* Affiche le menu */
    int nbPg = menu.nbItems / (LCD_ROWS_ - 1);
    int curPg = selectedMenuItem / ( LCD_ROWS_ - 1);   // [0 ... ]
    
    int nbItemsToShow = min(LCD_ROWS_ - 1, menu.nbItems - curPg * (LCD_ROWS_ - 1));
    
    lcd->clear();
    //scrollBarVert((int) ( menu.nbItems / selectedMenuItem ), LCD_COLUMNS_ - 1, 0, LCD_ROWS_);
    
    centerText(menu.prompt);

    String msg;
    
    msg = curPg + 1;
    msg += "/";
    msg += nbPg + 1;
    
    lcd->setCursor(LCD_COLUMNS_ - 3, 0);
    lcd->print(msg);
    
    for(int ii = 0; ii < nbItemsToShow; ++ii)
    {
      int itemArr = ( LCD_ROWS_ - 1 ) * curPg + ii;
      
      lcd->setCursor(0, ii + 1);
      msg = itemArr + 1;
      msg += itemArr == selectedMenuItem ? ")>" : ") ";
      msg +=  menu.items[ itemArr ];
    
      lcd->print( msg );
    }
    
    /* Attend l'appui sur un bouton */
    while((buttonPressed = readPushButton()) == BP_NONE){}
 
    /* Gére l'appui sur le bouton */
    switch(buttonPressed) {
    case BP_UP: // Bouton haut = choix précédent
 
      /* Si il existe un choix précédent */
      if(selectedMenuItem > 0) {
 
        /* Passe au choix précédent */
        selectedMenuItem--;
      }
      break;
 
    case BP_DOWN: // Bouton bas = choix suivant
 
      /* Si il existe un choix suivant */
      if(selectedMenuItem < (menu.nbItems - 1)) {
 
        /* Passe au choix suivant */
        selectedMenuItem++;
      }
      break;
 
    case BP_LEFT: // Bouton gauche = sorti du menu
      shouldExitMenu = true;
      break;
 
    case BP_SELECT: //
    case BP_RIGTH:  // Bouton droit ou SELECT = validation du choix
      menu.callbackFnct(selectedMenuItem);
      break;
   }
  }
}

/**
 * \details Displays a scroll bar at column/row with height and percentage.
 * If you are not very interested in the inner working of this library, this is not for you.
 * This is the only function in phi_prompt library to use custom characters.
  * If you used custom characters and want to use this function or a long message with scroll bar, run the init again to reinitialize custom characters.
 * \param percentage This goes between 0 and 99, representing the location indicator on the bar.
 * \param column This is the column location of the scroll bar's top.
 * \param row This is the row location of the scroll bar's top.
 * \param v_height This is the height of the bar in number of rows.
 */
void GUI::scrollBarVert(byte percent, byte column, byte row, byte v_height)
{
  int mapped;
  if (percent>99) percent=99;
  mapped=(int)(v_height*2-2)*percent/100; // This is mapped position, 2 per row of bar.
  for (byte i=0;i<v_height;i++)
  {
    lcd->setCursor(column,row+i);
    if (i==(mapped+1)/2)
    {
      if (i==0)
      {
        lcd->write((uint8_t)0);
      }
      else if (i==v_height-1)
      {
        lcd->write(5);
      }
      else
      {
        if (mapped+1==(mapped+1)/2*2) lcd->write(2);
        else lcd->write(3);
      }
    }
    else
    {
      if (i==0)
      {
        lcd->write(1);
      }
      else if (i==v_height-1)
      {
        lcd->write(4);
      }
      else
      {
        lcd->write(' ');
      }
    }
  }
}

/** Affiche le choix de l'utilisateur */
void GUI::displayChoice(byte selectedMenuItem, const char** items)
{ 
  /* Affiche le choix de l'utilisateur */
  lcd->clear();
  lcd->print(F("Z'avez choisi :"));
  lcd->setCursor(0, 1);
  lcd->print(items[selectedMenuItem]);
 
  /* Attend l'appui sur le bouton gauche ou SELECT */
  byte buttonPressed;
  do {
    buttonPressed = readPushButton();
  } 
  while(buttonPressed != BP_LEFT && buttonPressed != BP_SELECT);
}

