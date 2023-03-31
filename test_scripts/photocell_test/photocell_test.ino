/* Photocell simple testing sketch. 
 
Connect one end of the photocell to 5V, the other end to Analog 0.
Then connect one end of a 10K resistor from Analog 0 to ground 
Connect LED from pin 11 through a resistor to ground 
For more information see http://learn.adafruit.com/photocells */
 
int photocellPin = 36;     // the cell and 10K pulldown are connected to a0
int photocellReading;     // the analog reading from the sensor divider
const int MAX_READING = 4096; // 2^12 - 1
const int MIN_READING = 800;
const int MAX_BRIGHTNESS = 15;
const int MIN_BRIGHTNESS = 0;
int translation;


void setup(void) {
  // We'll send debugging information via the Serial monitor
  Serial.begin(115200);   
}
 
void loop(void) {
  photocellReading = analogRead(photocellPin);
  if (photocellReading <= MIN_READING) {
    translation = MIN_BRIGHTNESS;
  } else {
    translation = map(photocellReading, MIN_READING, MAX_READING, MIN_BRIGHTNESS, MAX_BRIGHTNESS+1);
  }
 
  Serial.print("Reading:");
  Serial.print(photocellReading);     // the raw analog reading
  Serial.print(",Translation:");
  Serial.println(translation);
 
  delay(100);
}