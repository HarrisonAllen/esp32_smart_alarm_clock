#include <WiFi.h>
#include <HTTPClient.h>

const char WIFI_SSID[] = "Cozy Cove";
const char WIFI_PASSWORD[] = "Prickly Mochi 1005";

String HOST_NAME = "http://alarm.lil-s.us";
String SNOOZE_PATH_NAME = "/snooze";
String STOP_PATH_NAME = "/stop";

const int snoozeButtonPin = 15;
const int stopButtonPin = 5;

int snoozeButtonState = HIGH;
int stopButtonState = HIGH;

void setup() {
  // put your setup code here, to run once:
    Serial.begin(115200);
    pinMode(snoozeButtonPin, INPUT_PULLUP);
    pinMode(stopButtonPin, INPUT_PULLUP);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.println("Connecting");
    while(WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
}

void loop() {
    int newSnoozeState = digitalRead(snoozeButtonPin);
    if (snoozeButtonState != newSnoozeState && newSnoozeState == HIGH) {
        snooze();
    }
    snoozeButtonState = newSnoozeState;
    
    int newStopState = digitalRead(stopButtonPin);
    if (stopButtonState != newStopState && newStopState == HIGH) {
        stop();
    }
    stopButtonState = newStopState;
    delay(40);
}

void snooze() {
    Serial.println("Snoozing...");
    HTTPClient http;

    http.begin(HOST_NAME + SNOOZE_PATH_NAME); //HTTP
    int httpCode = http.GET();

    // httpCode will be negative on error
    if(httpCode > 0) {
        // file found at server
        if(httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            Serial.println(payload);
        } else {
        // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        }
    } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
}

void stop() {
    Serial.println("Stopping...");
    HTTPClient http;

    http.begin(HOST_NAME + STOP_PATH_NAME); //HTTP
    int httpCode = http.GET();

    // httpCode will be negative on error
    if(httpCode > 0) {
        // file found at server
        if(httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            Serial.println(payload);
        } else {
        // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        }
    } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
}