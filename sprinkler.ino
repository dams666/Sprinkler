#include <Wire.h>  // Comes with Arduino IDE
// Get the LCD I2C Library here: 
// https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads
// Move any other LCD libraries to another folder or delete them
// See Library "Docs" folder for possible commands etc.
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

//#define WITH_SERIAL
#undef WITH_SERIAL

#define RELAY_OFF HIGH
#define RELAY_ON LOW
  
#ifdef WITH_SERIAL
#define DEBUG_PRINT(msg)\
Serial.print((msg));                              
#else
#define DEBUG_PRINT(msg) 
#endif

#ifdef WITH_SERIAL
#define DEBUG_PRINTLN(msg)\
Serial.println(msg);      \
delay(50);                      
#else
#define DEBUG_PRINTLN(msg) 
#endif

LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

// the setup routine runs once when you press reset:
void setup()
{
  
  lcd.begin(20,4);         // initialize the lcd for 20 chars 4 lines
  
//#ifdef WITH_SERIAL
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
//#endif

  initSprinkler();
}

// the loop routine runs over and over again forever:
void loop()
{        
  action();
}






