// TODO:
// - Implement buttons (also in files)
// - Implement database
// - Implement multiple alarms
// - Add more settings to alarms
// - Juice up the alarms page
// - TODO: add an offline mode
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <AsyncTCP.h>
#include <Sound.h>
#include <ClockController.h>
#include <nOOds.h>
#include <AlarmObject.h>
#include <ESPAsyncWebSrv.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_Trellis.h"
#include "SD.h"
#include "FS.h"
#include <Arduino_JSON.h>

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

// Initialize Alarm Object
AlarmObject alarmObject = AlarmObject();

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

  // Initialize a NTPClient to get time
  timeClient.begin();
  timeClient.setTimeOffset(-14400); // GMT -4, hours -> seconds
  fetchTime();
  lastMinute = clockController.getMinute();

  // Initialize websocket
  initWebSocket();
  
  // Serve Webpage
  setupWebPages();
  server.begin();

  // Start sound
  sound.begin();

  // Setup trellis
  trellis.begin(0x71);

  // nOOds
  noodsTimer = millis();
}

void loop() {
  clockController.loop();
  noodsLoop();
  trellisLoop();
  sound.loop();
  if (clockController.needsTimeUpdate()) {
      fetchTime();
  }
  if (clockController.getSecond() != lastSecond) {
    if (clockController.getMinute() != lastMinute) {
      lastMinute = clockController.getMinute();
      if (alarmObject._enabled && alarmObject.checkTime(&clockController)) {
          playAlarm();
      }
    }
    lastSecond = clockController.getSecond();
    notifyClients(getData());
  }
  ws.cleanupClients();
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

void setAlarm(String alarmString) {
    Serial.print("Alarm received: ");
    Serial.println(alarmString);

    int alarmHour, alarmMinute;
    alarmHour = alarmString.substring(0, 2).toInt();
    alarmMinute = alarmString.substring(3).toInt();
    alarmObject.setTime(alarmHour, alarmMinute);
    alarmObject.setEnabled(true);
    Serial.printf("Alarm set for %d:%d\n", alarmHour, alarmMinute);
}

void setAlarm(int minuteOffset) {
    int alarmMinute = clockController.getMinute() + minuteOffset;
    int alarmHour = clockController.getHour();
    if (alarmMinute >= 60) {
        alarmMinute -= 60;
        alarmHour += 1;
        if (alarmHour > 23) {
            alarmHour = 0;
        }
    }
    alarmObject.setTime(alarmHour, alarmMinute);
    alarmObject.setEnabled(true);
    Serial.printf("Alarm set for %d:%d\n", alarmHour, alarmMinute);
}

void setAlarmEnabled(String alarmEnabledString) {
    Serial.print("Alarm enable status: ");
    if (alarmEnabledString == "true") {
        Serial.println("Enabled");
    } else {
        Serial.println("Disabled");
        if (alarmObject._alarmPlaying) {
            Serial.println("Stopping alarm");
            sound.stop();
            alarmObject._alarmPlaying = false;
            setTrellisAlarmActive(false);
        }
    }
    alarmObject.setEnabled(alarmEnabledString == "true");
}

void playAlarm() {
    Serial.println("Alarm triggered!");
    sound.setSoundFile(alarmObject._alarmFilename);
    sound.setRepeating(true);
    sound.play();
    alarmObject._alarmPlaying = true;
    setTrellisAlarmActive(true);
}

void snooze() {
    if (alarmObject._alarmPlaying) {
        Serial.println("Snoozing alarm");
        sound.stop();
        alarmObject._alarmPlaying = false;
        setAlarm(alarmObject._snoozeDuration);
        setTrellisAlarmActive(false);
    }
}