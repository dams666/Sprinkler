
#include <MOD_moisture_sensors.h>

MOD_moistureSensors_* moistureSensors;

void setup() {
  moistureSensors = new MOD_moistureSensors_();
  Serial.begin(9600);
  Serial.println("Starting...");
}

void loop() {
  moistureSensors->start();
  moistureSensors->execute();
  moistureSensors->stop();
  delay(5000); // wait 1h

} 
