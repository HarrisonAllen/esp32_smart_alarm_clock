
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <AsyncTCP.h>
#include <Sound.h>
#include <ClockController.h>
#include <nOOds.h>
#include <ESPAsyncWebSrv.h>
#include "Adafruit_LEDBackpack.h"
#include "7-Segment-ASCII_HEX.h"
#include "Adafruit_Trellis.h"
#include "SD.h"
#include "FS.h"

// Wifi credentials
const char* ssid     = "Cozy Cove";
const char* password = "Prickly Mochi 1005";

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Initialize clock display and controller
Adafruit_7segment clockDisplay = Adafruit_7segment();
const int photocellPin = 36;
ClockController clockController(&clockDisplay, photocellPin);

// nOOds fiber
const int NOODS_PIN = 2;
nOOds nood(NOODS_PIN);
const int NOODS_UPDATE_RATE = 25;
long noodsTimer;

// I2S Connections
#define I2S_DOUT      27
#define I2S_BCLK      14
#define I2S_LRC       12

// Sound
Audio audio;
Sound sound(&audio);

// Trellis
#define numKeys 16
Adafruit_Trellis matrix0 = Adafruit_Trellis();
Adafruit_TrellisSet trellis =  Adafruit_TrellisSet(&matrix0);
long trellisTimer;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Local sketch variables
long wifiTimer;
int hour, minute, second, day;

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
  
  // Connect to wifi
  Serial.print("Connecting to ");
  Serial.print(ssid);
  WiFi.begin(ssid, password);
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

  // Serve Webpage
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SD, "/webserver/index.html", "text/html");
  });
  server.serveStatic("/", SD, "/webserver/");
  server.begin();

  // Start sound
  sound.begin();

  // Setup trellis
  trellis.begin(0x71);
  // Turn on all LEDs
  for (uint8_t i=0; i<numKeys; i++) {
    trellis.setLED(i);
  }
  trellis.writeDisplay();
  delay(250);
  // Turn off all LEDs
  for (uint8_t i=0; i<numKeys; i++) {
    trellis.clrLED(i);
  }
  trellis.writeDisplay();

  // Initialize a NTPClient to get time
  timeClient.begin();
  timeClient.setTimeOffset(-14400); // GMT -4, hours -> seconds
  fetchTime();

  // nOOds
  noodsTimer = millis();
}

void loop() {
  clockController.loop();
  noodsLoop();
  trellisLoop();
  sound.loop();
}

void noodsLoop() {
  if (millis() - noodsTimer > NOODS_UPDATE_RATE) {
    nood.setBrightness(calculateNoodsBrightness(analogRead(photocellPin)));
    noodsTimer = millis();
  }  
}

uint8_t calculateNoodsBrightness(int photocellReading) {
  return map(photocellReading, 0, MAX_READING, MIN_NOODS_BRIGHTNESS, MAX_NOODS_BRIGHTNESS+1);
}

void trellisLoop() {
  if (millis() - trellisTimer > 30) {
    // If a button was just pressed or released...
    if (trellis.readSwitches()) {
      // go through every button
      for (uint8_t i=0; i<numKeys; i++) {
        // if it was pressed, turn LED on
        if (trellis.justPressed(i)) {
          Serial.print("v"); Serial.println(i);
          trellis.setLED(i);
          sound.setVolume(i+5);
          sound.playOnce("/audio/pop.mp3");
        } 
        // if it was released, turn LED off
        if (trellis.justReleased(i)) {
          Serial.print("^"); Serial.println(i);
          trellis.clrLED(i);
        }
      }
      // tell the trellis to set the LEDs we requested
      trellis.writeDisplay();
    }
    trellisTimer = millis();
  }
}

void fetchTime() {
    if (timeClient.update()) {
        hour = timeClient.getHours();
        minute = timeClient.getMinutes();
        second = timeClient.getSeconds();
        day = timeClient.getDay();
        Serial.println("Fetched time: " + timeClient.getFormattedTime());
        clockController.setTime(hour, minute, second, day);
    } else {
        Serial.println("Failed to fetch time");
        clockController.ignoreTimeUpdate();
    }
}
