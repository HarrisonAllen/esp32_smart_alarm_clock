#include <ClockController.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <AlarmObject.h>
#include <Sound.h>
#include <ESPAsyncWebSrv.h>

// Replace with your network credentials
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

// Initialize sound
Audio audio;
Sound sound(&audio);

// WebServer Stuff
void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}
AsyncWebServer server(80);
const char* PARAM_INPUT_1 = "alarm_input";
const char* PARAM_INPUT_2 = "alarm_enabled";
bool alarmUpdated;

// String processor for webpage
char placeholderCharArray[15];
String processor(const String& var){
  if (var == "TIME_STRING"){
    return clockController.generateDisplayTime();
  } else if (var == "ALARM_HOUR"){
    sprintf(placeholderCharArray, "%02d", alarmObject._hour);
    return String(placeholderCharArray);
  } else if (var == "ALARM_MINUTE"){
    sprintf(placeholderCharArray, "%02d", alarmObject._minute);
    return String(placeholderCharArray);
  } else if (var == "ALARM_ENABLED") {
    sprintf(placeholderCharArray, "%s", alarmObject._enabled ? "checked" : "");
    return String(placeholderCharArray);
  } else if (var == "ALARM_UPDATE") {
      if (alarmUpdated) {
          alarmUpdated = false;
          return String("<p style=\"color:green\">Alarm successfully updated!</p>");
      }
      return String();
  }
  return String();
}

// Local Sketch Variables
long wifiTimer;
int hour, minute, second, day;
int lastMinute;
bool alarmPlaying;

void setup() {
    Serial.begin(115200);

    // Initialize clock
    clockController.begin();
    clockController.displayLoading();

    // Start microSD Card
    if(!SD.begin())
    {
        Serial.println("Error accessing microSD card!");
        clockController.displayError("E Sd");
        while(true); 
    }

    // Start sound
    sound.begin();

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
    alarmObject.setEnabled(false);
    lastMinute = clockController.getMinute();

    // Initialize server
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SD, "/webserver/alarm.html", "text/html", false, processor);
    });
    server.serveStatic("/", SD, "/webserver/");

    // Receive an HTTP GET request at <ESP_IP>/get?alarm_input=<time>&alarm_enabled=<bool>
    server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
        // GET alarm_input value on <ESP_IP>/get?alarm_input=<time>
        if (request->hasParam(PARAM_INPUT_1)) {
            setAlarm(request->getParam(PARAM_INPUT_1)->value());
        }
        // GET alarm_enabled value on <ESP_IP>/get?alarm_enabled=<bool>
        if (request->hasParam(PARAM_INPUT_2)) {
            setAlarmEnabled(request->getParam(PARAM_INPUT_2)->value());
        } else {
            setAlarmEnabled("false");
        }
        alarmUpdated = true;
        request->redirect("/");
    });
    server.onNotFound(notFound);
    server.begin();
}

void loop() {
    clockController.loop();
    sound.loop();
    if (clockController.needsTimeUpdate()) {
        fetchTime();
    }
    if (clockController.getMinute() != lastMinute) {
        Serial.printf("%d -> %d\n", clockController.getMinute(), lastMinute);
        lastMinute = clockController.getMinute();
        if (alarmPlaying) {
            Serial.println("Snoozing alarm");
            sound.stop();
            alarmPlaying = false;
            setAlarm(alarmObject._snoozeDuration);
        }
        if (alarmObject._enabled && alarmObject.checkTime(&clockController)) {
            playAlarm();
        }
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

void setAlarmEnabled(String alarmEnabledString) {
    Serial.print("Alarm enable status: ");
    if (alarmEnabledString == "true") {
        Serial.println("Enabled");
    } else {
        Serial.println("Disabled");
    }
    alarmObject.setEnabled(alarmEnabledString == "true");
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

void playAlarm() {
    Serial.println("Alarm triggered!");
    sound.setSoundFile(alarmObject._alarmFilename);
    sound.setRepeating(true);
    sound.setVolume(1);
    sound.play();
    alarmPlaying = true;
}