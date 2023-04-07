// TODO:
// - Add some files, especially for web server
// - Implement websockets!
// - Implement buttons (also in files)
// - Implement database
// - Implement multiple alarms
// - Add more settings to alarms
// - Juice up the alarms page
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
#include "7-Segment-ASCII_HEX.h"
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
long trellisTimer;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
// Create a WebSocket object
AsyncWebSocket ws("/ws");
void notFound(AsyncWebServerRequest *request) {
    Serial.println("Uh oh, 404 error!");
    request->send(SD, "/webserver/404.html", "text/html");
}

// Local sketch variables
long wifiTimer;
int hour, minute, second, day;
int lastMinute, lastSecond;
bool alarmPlaying;
String message = "";

// Json variable to hold data
JSONVar jsonData;
String getData() {
    jsonData["currentTime"] = clockController.generateDisplayTime(true);
    jsonData["alarmTime"] = alarmObject.generateDisplayAlarm();
    jsonData["alarmEnabled"] = alarmObject._enabled ? "true" : "false";
    jsonData["alarmActive"] = alarmPlaying ? "true" : "false";

    String jsonString = JSON.stringify(jsonData);
    return jsonString;
}

void notifyClients(String dataString) {
    ws.textAll(dataString);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        data[len] = 0;
        message = (char*)data;
        Serial.println("Received: " + message);
        if (message[0] == 'a') {
            Serial.println("New alarm time: " + message.substring(1));
            setAlarm(message.substring(1));
            notifyClients(getData());
        }
        if (message[0] == 'e') {
            Serial.println("Alarm enabled: " + message.substring(1));
            setAlarmEnabled(message.substring(1));
            notifyClients(getData());
        }
        if (message[0] == 's') {
            Serial.println("Snooze received");
            if (alarmPlaying) {
                Serial.println("Snoozing alarm");
                sound.stop();
                alarmPlaying = false;
                setAlarm(alarmObject._snoozeDuration);
                notifyClients(getData());
            }
        }
        if (message == "getData") {
            notifyClients(getData());
        }
    }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
            break;
        case WS_EVT_DISCONNECT:
            Serial.printf("WebSocket client #%u disconnected\n", client->id());
            break;
        case WS_EVT_DATA:
            handleWebSocketMessage(arg, data, len);
            break;
        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            break;
    }
}

void initWebSocket() {
    ws.onEvent(onEvent);
    server.addHandler(&ws);
}

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
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SD, "/webserver/index.html", "text/html");
  });
  server.on("/alarm", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SD, "/webserver/alarm.html", "text/html");
  });
  server.serveStatic("/", SD, "/webserver/static/");
  server.onNotFound(notFound);
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

void setAlarm(String alarmString) {
    Serial.print("Alarm received: ");
    Serial.println(alarmString);

    int alarmHour, alarmMinute;
    alarmHour = alarmString.substring(0, 2).toInt();
    alarmMinute = alarmString.substring(3).toInt();
    alarmObject.setTime(alarmHour, alarmMinute);
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
        if (alarmPlaying) {
            Serial.println("Stopping alarm");
            sound.stop();
            alarmPlaying = false;
        }
    }
    alarmObject.setEnabled(alarmEnabledString == "true");
}

void playAlarm() {
    Serial.println("Alarm triggered!");
    sound.setSoundFile(alarmObject._alarmFilename);
    sound.setRepeating(true);
    sound.setVolume(10);
    sound.play();
    alarmPlaying = true;
}