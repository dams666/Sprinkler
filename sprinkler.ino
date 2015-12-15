/*
  DigitalReadSerial
 Reads a digital input on pin 2, prints the result to the serial monitor

 This example code is in the public domain.
 */

int mainValvePin = 12;

int humPins[] = {0, 1, 2, 3, 4};
int valvePins[] = {11, 10, 9, 8, 7};

int state[] = {-1, -1, -1, -1, -1};         // current state of the machine
int sensorCount = 3;

 
// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  //Serial.begin(9600);
  // make the pushbutton's pin an input:
  pinMode(mainValvePin, OUTPUT);

  // the array elements are numbered from 0 to (pinCount - 1).
  // use a for loop to initialize each pin as an output:
  for (int thisPin = 0; thisPin < sensorCount; thisPin++) {
    pinMode(humPins[thisPin], INPUT);
    pinMode(valvePins[thisPin], OUTPUT);
  }
  
}

// the loop routine runs over and over again forever:
void loop() {
  // read the input pin:
  String aa;
  
  for (int thisPin = 0; thisPin < sensorCount; thisPin++)
  {
    int curState = digitalRead(humPins[thisPin]);

    aa += String(curState);
    aa+= "";

    if (curState != state[thisPin])
    {
      if (curState == HIGH) // hulidité trop faible
      {
        String msg =  "Humidité faible => arrosage valve ";
        msg += String(thisPin);
        //Serial.println(msg);
        
        digitalWrite(valvePins[thisPin], LOW); // activation du relais
        delay(500);
      } else {
  
        String msg =  "Fermeture valve ";
        msg += String(thisPin);
        //Serial.println(msg);
        
        
        digitalWrite(valvePins[thisPin], HIGH);
      }
    }
  
    state[thisPin] = curState;

    if (isValveOpened())
    {
      digitalWrite(mainValvePin, LOW); // activation du relais
    } else {
      digitalWrite(mainValvePin, HIGH); // extinction du relais
    }

  }

  //Serial.println(aa);
  delay(1000);        // delay in between reads for stability
}


bool isValveOpened() 
{
    for (int thisPin = 0; thisPin < sensorCount; thisPin++)
    {
      if (state[thisPin] == 1)
        return true;
    }
    return false;
}



