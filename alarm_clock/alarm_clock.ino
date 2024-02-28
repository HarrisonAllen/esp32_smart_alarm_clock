/*
How to use:
- Select "ESP32 Dev Module" as the board type
- Put the files in sd_card onto the device sd card, that's also where the webpages live
*/
/* TODO
    Hard
    - Juice up alarms page
*/
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <AsyncTCP.h>
#include <Sound.h>
#include <ClockController.h>
#include <AlarmObject.h>
#include <ESPAsyncWebSrv.h>
#include "Adafruit_LEDBackpack.h"
#include "SD.h"
#include "FS.h"
#include <Arduino_JSON.h>

// Wifi credentials
String ssid = "";
String password = "";

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Initialize clock display and controller
Adafruit_7segment clockDisplay = Adafruit_7segment();
const int photocellPin = 36;
ClockController clockController(&clockDisplay, photocellPin);

// I2S Connections
#define I2S_DOUT      27
#define I2S_BCLK      14
#define I2S_LRC       12

// Sound
Audio audio;
Sound sound(&audio);

// Initialize Alarm Objects
#define NUM_ALARMS 1
AlarmObject alarmObject = AlarmObject();

// Local sketch variables
long wifiTimer;
int hour, minute, second, day;
int lastMinute, lastSecond;
String message = "";
// Json variable to hold data
JSONVar jsonData;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
// Create a WebSocket object
AsyncWebSocket ws("/ws");

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Setup clock controller
  clockController.begin();
  clockController.displayLoading();

  // Start microSD Card
  if(!SD.begin())
  {
    Serial.println("Error accessing microSD card!");
    clockController.displayError("E Sd");
    while(true); 
  }

  // Setup alarms
  alarmObject.init(NUM_ALARMS, &sound, &clockController, &timeClient);

  // Get wifi info from SD card
  if (SD.exists("/wifi.info")) {
    File wifiFile = SD.open("/wifi.info");
    if (wifiFile) {
      char curChar;
      bool ssidComplete = false;
      while (wifiFile.available()) {
        curChar = (char)wifiFile.read();
        if (curChar == '\n') {
          ssidComplete = true;
          continue;
        }
        if (curChar == '\r') {
            continue;
        }
        if (ssidComplete) {
          password += curChar;
        } else {
          ssid += curChar;
        }
      }
      wifiFile.close();
      Serial.println("Successfully read wifi info from SD");
    } else {
      Serial.println("Failed to read wifi info from SD");
    }
  }

  // Connect to wifi
  Serial.print("Connecting to ");
  Serial.print(ssid);
  WiFi.begin(ssid.c_str(), password.c_str());
  wifiTimer = millis();
  while (true) {
    if (millis() - wifiTimer > 500) {
      if (WiFi.status() == WL_CONNECTED) break;
      Serial.print(".");
      wifiTimer = millis();
    }
    clockController.loop();
  }
  // Print local IP address
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  clockController.displayIP(WiFi.localIP());

  // Initialize a NTPClient to get time
  timeClient.begin();
  fetchTime();
  lastMinute = clockController.getMinute();

  // Initialize websocket
  initWebSocket();
  
  // Serve Webpage
  setupWebPages();
  server.begin();

  // Start sound
  sound.begin();
}

bool isNewSecond() {
    if (clockController.getSecond() != lastSecond) {
        lastSecond = clockController.getSecond();
        return true;
    }
    return false;
}

bool isNewMinute() {
    if (clockController.getMinute() != lastMinute) {
        lastMinute = clockController.getMinute();
        return true;
    }
    return false;
}

void loop() {
    clockController.loop();
    sound.loop();
    if (isNewMinute()) {
        if (alarmObject._timeOffsetChanged) {
            alarmObject._timeOffsetChanged = !fetchTime();
        }
        if (alarmObject.checkAlarms()) {
            notifyClients(getData());
        }
    }
    if (clockController.needsTimeUpdate()) {
        fetchTime();
    }
    ws.cleanupClients();
}

bool fetchTime() {
    if (timeClient.update()) {
        hour = timeClient.getHours();
        minute = timeClient.getMinutes();
        second = timeClient.getSeconds();
        day = timeClient.getDay();
        Serial.println("Fetched time: " + timeClient.getFormattedTime());
        clockController.setTime(hour, minute, second, day);
        return true;
    } else {
        Serial.println("Failed to fetch time");
        clockController.ignoreTimeUpdate();
        return false;
    }
}