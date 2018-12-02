#include "Gui.h"

#include <LiquidCrystal_I2C.h>
#include <IRremote.h>

GUI::GUI(uint8_t lcd_i2c_addr, uint8_t rc_pin)
{
 
  /* Objet LCD sur les broches utilisées par la shield LCD DFrobots */
  lcd = new LiquidCrystal_I2C(lcd_i2c_addr, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

  irDetect = new IRrecv(rc_pin);
  irIn = new decode_results;
  
  sleepingMode = false;
  lasTimeBtnCall = 0;
  /* Configuration du LCD */
  lcd->begin(20, 4);
  
  irDetect->enableIRIn(); // Start the Receiver
}


/** Fonction retournant le bouton appuyé (s’il y en a un). */
Button_t GUI::readPushButton(void)
{ 

  if (sleepingMode)
  {
    if  (irDetect->decode(irIn))
    {
      sleepingMode = false;
      //lcd->backlight();
    lcd->on();
  } 
    
  return BP_NONE;
    
  } else
  {
    if  (!irDetect->decode(irIn))
    {
      if (millis() - lasTimeBtnCall > LCD_SLEEPING_TIMEOUT)
      {
        sleepingMode = true;
        //lcd->noBacklight();
    lcd->off();
      }
      return BP_NONE;
    }
  }
   
  Button_t res = BP_NONE;
  
  lasTimeBtnCall = millis();
  
  
  switch(irIn->value)
  {

  case 0xFF629D:  
    res = BP_UP;
  break;
  case 0xFF22DD:  
    //Serial.println("Left Arrow"); 
    res = BP_LEFT;
  break;
  case 0xFF02FD:  
    //Serial.println("OK"); 
    res = BP_OK;
  break;
  case 0xFFC23D:  
    //Serial.println("Right Arrow"); 
    break;
  res = BP_RIGHT;
  case 0xFFA857:  
    //Serial.println("Down Arrow"); 
    res = BP_DOWN;
  break;
  case 0xFF6897:  
    //Serial.println("1"); 
    break;

  case 0xFF9867:  
    //Serial.println("2"); 
    break;

  case 0xFFB04F:  
    //Serial.println("3"); 
    break;

  case 0xFF30CF:  
    //Serial.println("4"); 
    break;

  case 0xFF18E7:  
    //Serial.println("5"); 
    break;

  case 0xFF7A85:  
    //Serial.println("6"); 
    break;

  case 0xFF10EF:  
    //Serial.println("7"); 
    break;

  case 0xFF38C7:  
    //Serial.println("8"); 
    break;

  case 0xFF5AA5:  
    //Serial.println("9"); 
    break;

  case 0xFF42BD:  
    //Serial.println("*"); 
    break;

  case 0xFF4AB5:  
    //Serial.println("0"); 
    break;

  case 0xFF52AD:  
    //Serial.println("#"); 
    break; 

  default: 
   break;

  }

  irDetect->resume(); 
  
  return res;
}





/**
 * \details This function displays a short message in SRAM centered with the display size.
 * This outputs a text center-aligned on the display depending on the display size.
 * Eg. center_text("Introduction") outputs ">>>>Introduction<<<<" on 20 column display.
 * \param src This is the string to be displayed.
 */
void GUI::centerText(const __FlashStringHelper* src)
{
  byte i=0;
  byte j=0;
  char msg[LCD_COLUMNS_+1];
  
  const byte len_src = strlen_P((const char *)src);

  if (src == NULL)
    return;
    
  msg[0] = '\0';
  
  for (i=0;i<LCD_COLUMNS_;i++)
  {
    if (i<LCD_COLUMNS_/2-(len_src-len_src/2)) strcat(msg, ">");
    else if (i>=LCD_COLUMNS_/2+len_src/2) strcat (msg, "<");
    else 
    {
      char cToStr[2];
      cToStr[0] = pgm_read_byte_near((const char *)src + j);
      cToStr[1] = '\0';
      
      strcat(msg,cToStr);
      j++;
    }
  }
  lcd->print(msg);
}

int GUI::displayIntPrompt(const __FlashStringHelper* msg, const __FlashStringHelper* unit,int start, int min, int max, int step)
{
  Button_t buttonPressed;      // Contient le bouton appuyé
  byte shouldExitMenu = false; // Devient true quand l'utilisateur veut quitter le menu
  int value = start;
  
  lcd->clear();
  lcd->setCursor(0, 0);
  lcd->print(msg);
  
  String txt;
  
  /* Tant que l'utilisateur ne veut pas quitter pas le menu */
  while(!shouldExitMenu)
  {
    if (value > max) value = max;
    if (value <min) value = min;
    
    txt = value;
    txt += F(" ");
    txt += unit;
    txt += F("   ");
    
    lcd->setCursor(6, 2);
    lcd->print(txt);
    
    /* Attend l'appui sur un bouton */
    while((buttonPressed = readPushButton()) == BP_NONE){delay(100);}
    
    /* Gére l'appui sur le bouton */
    switch(buttonPressed)
    {
      case BP_UP: // Bouton haut = choix précédent 
      value += step;
      break;
      case BP_DOWN: // Bouton bas = choix suivant
      value-=step;
      break;
      case BP_CANCEL: // Bouton gauche = sorti du menu
      shouldExitMenu = true;
      break;
     
      case BP_OK: //
      shouldExitMenu = true;
      break;
     }
  
  }
  
  return value;
}

bool GUI::displayYNPrompt(const __FlashStringHelper * msg, bool dftYes)
{
  Button_t buttonPressed;      // Contient le bouton appuyé
  byte shouldExitMenu = false; // Devient true quand l'utilisateur veut quitter le menu
  
  lcd->clear();
  lcd->setCursor(0, 0);
  lcd->print(msg);

  bool isYes = dftYes;  
  
  /* Tant que l'utilisateur ne veut pas quitter pas le menu */
  while(!shouldExitMenu)
  {
    lcd->setCursor(6, LCD_ROWS_ - 1);
    if (isYes)
    {
      lcd->print(F(">>YES<<   NO  "));
    } else {
      lcd->print(F("  YES   >>NO<<"));
    }
    
    /* Attend l'appui sur un bouton */
    while((buttonPressed = readPushButton()) == BP_NONE){delay(100);}
   
    /* Gére l'appui sur le bouton */
    switch(buttonPressed)
    {
      case BP_UP:
      case BP_DOWN:
      case BP_LEFT:
      case BP_RIGHT:
        isYes = !isYes;
        break;
      case BP_CANCEL:
        shouldExitMenu = true;
        break;
      case BP_OK:
        shouldExitMenu = true;
        break;
     }
  }

  return isYes;
}
/** Affiche le menu passé en argument */
void GUI::displayMenu(const Menu_t &menu)
{ 
  /* Variable pour le menu */
  byte selectedMenuItem = 0;   // Choix selectionné
  byte shouldExitMenu = false; // Devient true quand l'utilisateur veut quitter le menu
  Button_t buttonPressed;      // Contient le bouton appuyé

  lcd->clear();
  //scrollBarVert((int) ( menu.nbItems / selectedMenuItem ), LCD_COLUMNS_ - 1, 0, LCD_ROWS_);
    
  centerText((const __FlashStringHelper*)menu.prompt);
    
  /* Tant que l'utilisateur ne veut pas quitter pas le menu */
  while(!shouldExitMenu)
  {
    /* Affiche le menu */
    int nbPg = menu.nbItems / (LCD_ROWS_ - 1);
    int curPg = selectedMenuItem / ( LCD_ROWS_ - 1);   // [0 ... ]
    
    int nbItemsToShow = min(LCD_ROWS_ - 1, menu.nbItems - curPg * (LCD_ROWS_ - 1));
    
    String msg;
    char buffer[LCD_COLUMNS_+1];
  
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
      msg += itemArr == selectedMenuItem ? F(")>") : F(") ");
    
    strcpy_P(buffer, (char*)pgm_read_word(menu.items + itemArr ));
      //msg +=  menu.items[ itemArr ];
    msg += buffer;
    
      int l = LCD_COLUMNS_ - msg.length();
      for (int jj = 0; jj< l ; ++jj) 
        msg+=" ";
        
      lcd->print( msg );
    }

    for(int ii = nbItemsToShow; ii < LCD_ROWS_ - 1; ++ii)
    {
      lcd->setCursor(0, ii + 1);
      msg = "";
      for (int jj = 0; jj< LCD_COLUMNS_; ++jj) 
          msg+=" ";
      lcd->print( msg );
    }
    
    /* Attend l'appui sur un bouton */
    while((buttonPressed = readPushButton()) == BP_NONE){delay(100);}
 
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
 
    case BP_CANCEL: // Bouton gauche = sorti du menu
      shouldExitMenu = true;
      break;
 
    case BP_OK: //
      menu.callbackFnct(selectedMenuItem);
      shouldExitMenu = true;
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
void GUI::displayText(String msg, const __FlashStringHelper* title, bool waitKey)
{ 
  char *running;
  char *token;

  running = strdup (msg.c_str());

  /* Affiche le choix de l'utilisateur */
  lcd->clear();
  //lcd->print(F("Votre choix:"));
  centerText(title);
  
  int jj = 1;
  while ((token = strsep (&running, "\n")))
  {
  LCD_PRINT(0, jj++, token);
  }
  delete[] running;
  
  if (waitKey)
  {
    /* Attend l'appui sur le bouton gauche ou SELECT */
    byte buttonPressed;
    do {
    buttonPressed = readPushButton();
    } 
    while(buttonPressed != BP_LEFT && buttonPressed != BP_OK);
  }
}

/** Affiche le choix de l'utilisateur */
void GUI::displayText(const __FlashStringHelper* msg, const __FlashStringHelper* title, bool waitKey)
{ 
  char running[4*(LCD_COLUMNS_+1)];
  //char *running;
  char *token;

  //running = strdup (msg);
  strcpy_P(running, (char*)pgm_read_word(msg));
  
  /* Affiche le choix de l'utilisateur */
  lcd->clear();
  //lcd->print(F("Votre choix:"));
  centerText(title);
  
  int jj = 1;
  //while ((token = strsep (&(char*)running, "\n")))
  {
  //LCD_PRINT(0, jj++, token);
  LCD_PRINT(0, jj++, running);
  }
  //delete[] running;
  
  if (waitKey)
  {
    /* Attend l'appui sur le bouton gauche ou SELECT */
    byte buttonPressed;
    do {
    buttonPressed = readPushButton();
    } 
    while(buttonPressed != BP_LEFT && buttonPressed != BP_OK);
  }
}



