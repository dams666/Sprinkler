//#include <Eepromutil.h>
#include <Gui.h>

#define STAT_LOG_SIZE 15
#define STAT_MSG_LEN 21 //LCD_COLUMNS_+ 1

#include <WaterStatsLogger.h>

#define LCD_I2C_ADDR 0x3F
#define RC_PIN 5

GUI*              __gui;
waterStatsLogger* __statsLogger;

#define LCD_CLEAR()\
__gui->lcd->clear();

#define LCD_PRINT(col, line, msg)\
__gui->lcd->setCursor(col,line);\
__gui->lcd->print(msg);\
delay(50);

void printTestResult(uint16_t expectedWaterings, unsigned long expectedMl, int expectedLogLine) {
  char text[STAT_LOG_SIZE][STAT_MSG_LEN];
  uint8_t len = 0;
  unsigned long totalMililitres = 0;
  uint16_t nbWaterings = 0;
  
  __statsLogger->readLogStats(text, len, totalMililitres, nbWaterings);
  
  Serial.print("Test waterings (should expect ");
  Serial.print(expectedWaterings);
  Serial.print("): ");
  Serial.println(nbWaterings);

  if (nbWaterings == expectedWaterings) {
    Serial.println("=> OK");
  } else {
    Serial.println("=> KO");
  }

  Serial.print("Test ml (should expect ");
  Serial.print(expectedMl);
  Serial.print(" ml): ");
  Serial.println(totalMililitres);

  if (totalMililitres == expectedMl) {
    Serial.println("=> OK");
  } else {
    Serial.println("=> KO");
  }
  /*
  Serial.print("Test logline (should expect ");
  Serial.print(expectedLogLine);
  Serial.print("): ");
  Serial.println(waterStats.watSessionLogLine);

  if (waterStats.watSessionLogLine == expectedLogLine) {
    Serial.println("=> OK");
  } else {
    Serial.println("=> KO");
  }
  */
  Serial.println("Log: ");
  
  for (int i = 0; i < len; ++i) {
    Serial.println(text[i]);
  } 

  __gui->displayText2(text, len, F("LOG"));
}

void setup() {

  __gui = new GUI(LCD_I2C_ADDR, RC_PIN);
  __statsLogger = new waterStatsLogger(0);
  
  Serial.begin(115200);  

  Serial.print("Clearing memory... ");
  __statsLogger->clearStats();
  //eeprom_erase_all(0);
  Serial.println("Done!");

  Serial.println("--------------------------------------------------------------");
  Serial.println("Test lecture / ecriture: 0 session d'arrosage");
  Serial.println("--------------------------------------------------------------");
  
  printTestResult(0, 0, 0);
  
  Serial.println("--------------------------------------------------------------");
  Serial.println("Test lecture / ecriture: 15 sessions d'arrosage");
  Serial.println("--------------------------------------------------------------");

  int mlSession;
  unsigned long long totalMl = 0;
  for (int i = 0; i < STAT_LOG_SIZE; ++i) {
    mlSession = 10*(i+1);
    totalMl += mlSession;
    __statsLogger->saveSessionStats(mlSession);
  }

  printTestResult(15, totalMl, 0);

  Serial.println("--------------------------------------------------------------");
  Serial.println("Test rotation log");
  Serial.println("--------------------------------------------------------------");

  __statsLogger->saveSessionStats(160);
  totalMl +=160;
  printTestResult(STAT_LOG_SIZE + 1, totalMl, 1);

  Serial.println("--------------------------------------------------------------");
  Serial.println("Test lecture / ecriture: 14 sessions d'arrosage pour supprimer l'historique ancien");
  Serial.println("--------------------------------------------------------------");

  for (int i = 0; i < STAT_LOG_SIZE - 1; ++i) {
    mlSession = 10*(i+1) + 160;
    totalMl += mlSession;
    __statsLogger->saveSessionStats(mlSession);
  }
  printTestResult(2 * STAT_LOG_SIZE, totalMl, 0);
  
  Serial.println("Done!");

}

void loop() {
  // put your main code here, to run repeatedly:

}
