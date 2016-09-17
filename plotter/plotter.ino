#include <Adafruit_ADS1015.h>

Adafruit_ADS1115 *ads;

int *bits;
double *volts;

void setup() {

  ads = new Adafruit_ADS1115 (0x4A);  /* Use this for the 16-bit version */

  bits      = new int[4];
  volts     = new double[4];
  
  memset (bits, 0, sizeof(int) * 4);
  memset (volts, 0, sizeof(double) * 4);

  // The ADC input range (or gain) can be changed via the following
  // functions, but be careful never to exceed VDD +0.3V max, or to
  // exceed the upper and lower limits if you adjust the input range!
  // Setting these values incorrectly may destroy your ADC!
  //                                                                ADS1015  ADS1115
  //                                                                -------  -------
  // ads->setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
     ads->setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  // ads->setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads->setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads->setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads->setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV

  ads->begin();

  Serial.begin(9600);
}

void loop() {

  for (int ii = 0; ii<4; ++ii)
  {
    bits[ii]   = ads->readADC_SingleEnded(ii);
    volts[ii]  = bits[ii] * 0.000125;
  }

    Serial.print(volts[0]);
    Serial.print(" ");
    Serial.print(volts[1]);
    Serial.print(" ");
    Serial.print(volts[2]);
    Serial.print(" ");
    Serial.println(volts[3]);

  delay(300000);

}
