
//#define WITH_SERIAL
#undef WITH_SERIAL

#define RELAY_OFF HIGH
#define RELAY_ON LOW
  
byte statusLed    = 13;

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






