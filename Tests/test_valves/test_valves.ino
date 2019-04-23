#include <ModuleConfig.h>
#include <Gui.h>

uint8_t pins[MAX_CHANNELS_];

uint8_t __curChannel; 

GUI * __gui;
char str [80];



void setup() {
  
   memset (pins, 0, sizeof(uint8_t) * MAX_CHANNELS_);
    
    if (MAX_CHANNELS_>0) pins[0] = VALVE_1_PIN;    
    if (MAX_CHANNELS_>1) pins[1] = VALVE_2_PIN;
    if (MAX_CHANNELS_>2) pins[2] = VALVE_3_PIN;
    if (MAX_CHANNELS_>3) pins[3] = VALVE_4_PIN;
    if (MAX_CHANNELS_>4) pins[4] = VALVE_5_PIN;
    if (MAX_CHANNELS_>5) pins[5] = VALVE_6_PIN;
   
    pinMode(VALVE_M_PIN, OUTPUT);
    digitalWrite(VALVE_M_PIN, VALVE_OFF); // fermeture de la valve
    /*
    for (uint8_t thisPin = 0; thisPin < MAX_CHANNELS_; ++thisPin)
    {
      pinMode(pins[thisPin], OUTPUT);
      digitalWrite(pins[thisPin], VALVE_OFF); // fermeture de la valve
    }
    */
  __curChannel = -1;

  __gui = new GUI(0x3F, 5);  
  
  delay(2000);
}

void loop()
{
digitalWrite(VALVE_M_PIN, VALVE_ON); // ouverture de la valve principale
delay(1000);
digitalWrite(VALVE_M_PIN, VALVE_OFF); // ouverture de la valve principale
delay(1000);
}

/*
void loop() { 
  
  if (__curChannel == -1)
  {
    digitalWrite(VALVE_M_PIN, VALVE_ON); // ouverture de la valve principale
  } else {
    digitalWrite(pins[__curChannel], VALVE_OFF);
  }
  
  if (++__curChannel >= MAX_CHANNELS_)
  {
    __curChannel = -1;
    digitalWrite(VALVE_M_PIN, VALVE_OFF); // fermeture de la valve principale
    
    sprintf_P(str,PSTR("Closing main valve "),__curChannel + 1);
    __gui->displayText(str, NULL, false);
    
  } else {

    sprintf_P(str, PSTR("Opening valve %i"),__curChannel + 1);
    __gui->displayText(str, NULL, false);
    
    digitalWrite(pins[__curChannel], VALVE_ON);
  }
  
  delay(5000);
  
}
*/
