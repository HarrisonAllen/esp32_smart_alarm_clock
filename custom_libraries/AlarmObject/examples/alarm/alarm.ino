#include <ClockController.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <AlarmObject.h>
#include <Sound.h>

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
    setAlarm(1);
    lastMinute = clockController.getMinute();
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