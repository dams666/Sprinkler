
#define WITH_SERIAL

#define NB_VALVES 6

byte statusLed    = 13;

// the setup routine runs once when you press reset:
void setup()
{
#ifdef WITH_SERIAL
  // initialize serial communication at 9600 bits per second:
  Serial.begin(38400);
#endif

  pinMode(statusLed, OUTPUT);
  digitalWrite(statusLed, LOW);  // We have an active-low LED attached

  initSprinkler();
}

// the loop routine runs over and over again forever:
void loop()
{        
  action();
}






