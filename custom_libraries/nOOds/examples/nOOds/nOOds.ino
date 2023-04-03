#include <nOOds.h>

const int NOODS_PIN = 2;
const int NOODS_UPDATE_RATE = 20;
bool increasing = true;
int brightness;
long timer;

nOOds nood(NOODS_PIN);

void setup() {
  Serial.begin(115200);
  Serial.println("nOOds example");
  nood.begin();
  nood.setBrightness(brightness);
}

void loop() {
  if ((millis() - timer > NOODS_UPDATE_RATE)) {
    if (increasing) {
      brightness = nood.brightnessUp();
      if (brightness >= MAX_NOODS_BRIGHTNESS) {
        increasing = false;
      }
      Serial.printf("Increased to %d\n", brightness);
    } else {
      brightness = nood.brightnessDown();
      if (brightness <= MIN_NOODS_BRIGHTNESS) {
        increasing = true;
      }
      Serial.printf("Decreased to %d\n", brightness);
    }
    timer = millis();
  }
}