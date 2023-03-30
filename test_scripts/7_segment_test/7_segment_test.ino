#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"\

// I2C address of the display.  Stick with the default address of 0x70
// unless you've changed the address jumpers on the back of the display.
#define DISPLAY_ADDRESS   0x70

// Create display and DS1307 objects.  These are global variables that
// can be accessed from both the setup and loop function below.
Adafruit_7segment clockDisplay = Adafruit_7segment();

int hour, minute;
int displayHour;
char displayBuffer[8];
int displayValue;
const int updateDelay = 100;
long clockTimer;

void setup() {
  // Setup function runs once at startup to initialize the display

  // Setup Serial port to print debug output.
  Serial.begin(115200);
  Serial.println("Clock starting!");

  // Setup the display.
  clockDisplay.begin(DISPLAY_ADDRESS);
  clockTimer = millis();
}

void loop() {
  if (millis() - clockTimer > updateDelay) {
    clockTimer = millis();
    minute += 1;
    if (minute > 59) {
      minute = 0;
      hour += 1;
      if (hour > 23) {
        hour = 0;
      }
    }

    displayTime();
  }
}

void displayTime() {
  bool am = true;
  displayHour = hour;
  if (displayHour >= 12) {
    displayHour -= 12;
    am = false;
  }
  if (displayHour == 0) {
    displayHour = 12;
  }
  displayValue = displayHour * 100 + minute;
  // sprintf(displayBuffer, "%02d%02d", displayHour, minute);
  Serial.printf("%02d:%02d %s - %d\n", displayHour, minute, am ? "AM" : "PM", displayValue);
  clockDisplay.print(displayValue, DEC);

  clockDisplay.writeDigitRaw(2, (am ? 0x04 : 0x08) | ((minute % 10 < 5) ? 0x02 : 0x00));

  uint8_t brightness = (uint8_t)(displayHour - 1);
  clockDisplay.setBrightness(brightness);
  clockDisplay.writeDisplay();
}

