#include <TimeLib.h>
#include <Eepromutil.h>
#include <Gui.h>

#define WITH_DS1307
#ifdef WITH_DS1307
#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#endif

#define STAT_LOG_SIZE 15

// LOG: Statistiques de consommation d'eau
// fichiers placés
typedef struct
{
  time_t dateTime; // 4 bytes date representation
  unsigned int mlUsed; // 2 bytes
   
} wateringSession_;

typedef struct 
{
  unsigned long totalMililitres;
  unsigned int  nbWaterings;

  wateringSession_ wateringSession[STAT_LOG_SIZE];
  unsigned short watSessionLogLine; // identifiant de ligne courant dans le tableau de log
  
} waterStatsChanStorage_;



void readCurChannelStats(waterStatsChanStorage_* waterStats)
{
   eeprom_read_bytes( 0, (byte*)(waterStats), sizeof(waterStatsChanStorage_)); 
}

void saveSessionStats(int totalMililitresSession)
{
  waterStatsChanStorage_ waterStats;

  readCurChannelStats(&waterStats);

  #ifdef WITH_DS1307
  tmElements_t tm;
  if (RTC.read(tm))
  {      
    waterStats.wateringSession[waterStats.watSessionLogLine].mlUsed = totalMililitresSession;
    waterStats.wateringSession[waterStats.watSessionLogLine].dateTime = makeTime(tm);

    if (++waterStats.watSessionLogLine >= STAT_LOG_SIZE)
      waterStats. watSessionLogLine = 0;
  }
  #endif
  
  waterStats.totalMililitres += totalMililitresSession;
  ++waterStats.nbWaterings;
  
  // enregistrement des stats
  eeprom_write_bytes( 0, 
                      (const byte*)(&waterStats),
                      sizeof(waterStatsChanStorage_)); 
}


void readLogStats(char text[20][LCD_COLUMNS_+1], int& len, waterStatsChanStorage_ & waterStats) {
 
  tmElements_t tm;
  
  readCurChannelStats(&waterStats);

  if (waterStats.nbWaterings <= STAT_LOG_SIZE) // pas de rotation de log
  {
    len = waterStats.nbWaterings;
    for (int i = 0; i < len; ++i)
    {
      breakTime(waterStats.wateringSession[i].dateTime,tm);
      sprintf_P(text[i], PSTR("%02d/%02d %02d:%02d %d ml"), tm.Day, tm.Month, tm.Hour, tm.Minute, waterStats.wateringSession[i].mlUsed);
    }
  } else { // rotation de log
    len = STAT_LOG_SIZE;
    for (int i = 0; i < len; ++i)
    {
      int ii = (waterStats.watSessionLogLine + i) % STAT_LOG_SIZE;
            
      breakTime(waterStats.wateringSession[ii].dateTime,tm);
      sprintf_P(text[i], PSTR("%02d/%02d %02d:%02d %d ml"), tm.Day, tm.Month, tm.Hour, tm.Minute, waterStats.wateringSession[ii].mlUsed);
    }
  }

}


void printTestResult(int expectedWaterings, unsigned long expectedMl, int expectedLogLine) {
  char text[20][LCD_COLUMNS_+1];
  int len = 0;
  memset (text, 0, sizeof(char) * 20 * (LCD_COLUMNS_ - 1));  

  waterStatsChanStorage_ waterStats;
  
  readLogStats(text, len, waterStats);
  
  Serial.print("Test waterings (should expect ");
  Serial.print(expectedWaterings);
  Serial.print("): ");
  Serial.println(waterStats.nbWaterings);

  if (waterStats.nbWaterings == expectedWaterings) {
    Serial.println("=> OK");
  } else {
    Serial.println("=> KO");
  }

  Serial.print("Test ml (should expect ");
  Serial.print(expectedMl);
  Serial.print(" ml): ");
  Serial.println(waterStats.totalMililitres);

  if (waterStats.totalMililitres == expectedMl) {
    Serial.println("=> OK");
  } else {
    Serial.println("=> KO");
  }

  Serial.print("Test logline (should expect ");
  Serial.print(expectedLogLine);
  Serial.print("): ");
  Serial.println(waterStats.watSessionLogLine);

  if (waterStats.watSessionLogLine == expectedLogLine) {
    Serial.println("=> OK");
  } else {
    Serial.println("=> KO");
  }
  
  Serial.println("Log: ");
  
  for (int i = 0; i < len; ++i) {
    Serial.println(text[i]);
  } 

}

void setup() {

  Serial.begin(115200);  

  Serial.print("Clearing memory... ");
  eeprom_erase_all(0);
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
    saveSessionStats(mlSession);
  }

  printTestResult(15, totalMl, 0);

  Serial.println("--------------------------------------------------------------");
  Serial.println("Test rotation log");
  Serial.println("--------------------------------------------------------------");

  saveSessionStats(160);
  totalMl +=160;
  printTestResult(STAT_LOG_SIZE + 1, totalMl, 1);

  Serial.println("--------------------------------------------------------------");
  Serial.println("Test lecture / ecriture: 14 sessions d'arrosage pour supprimer l'historique ancien");
  Serial.println("--------------------------------------------------------------");

  for (int i = 0; i < STAT_LOG_SIZE - 1; ++i) {
    mlSession = 10*(i+1);
    totalMl += mlSession;
    saveSessionStats(mlSession);
  }
  printTestResult(2 * STAT_LOG_SIZE, totalMl, 0);
  
  Serial.println("Done!");

}

void loop() {
  // put your main code here, to run repeatedly:

}
