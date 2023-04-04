/* clock_demo.ino - Just a short demo that shows off:
    - Loading Screen
    - Error message
    - IP Address
    - Time
*/
#include <ClockController.h>

// Initialize clock display and controller
Adafruit_7segment clockDisplay = Adafruit_7segment();
const int photocellPin = 36;
ClockController clockController(&clockDisplay, photocellPin);

// Variables for demo
long loopTimer;
const uint8_t testIP[] = {192, 168, 0, 159};

void setup() {
    Serial.begin(115200);

    clockController.begin();
    loopTimer = millis();
    clockController.displayMessage("HELO", 5000);
}

void loop() {
    clockController.loop();
    if (millis() - loopTimer > 10000) {
        loopTimer = millis();
        switch (clockController.getState()) {
            case cs_init:
                // Shouldn't be here
                break;
            case cs_loading:
                Serial.println("Switching from loading to error");
                clockController.displayError("E HI");
                break;
            case cs_error:
                Serial.println("Switching from error to IP");
                clockController.displayIP(IPAddress(testIP));
                break;
            case cs_ip:
                // Shouldn't get here
                break;
            case cs_message:
                // Shouldn't get here
                break;
            case cs_time:
                Serial.println("Switching from time to loading");
                clockController.displayLoading();
                break;
            default:
                break;
        }
    }
}