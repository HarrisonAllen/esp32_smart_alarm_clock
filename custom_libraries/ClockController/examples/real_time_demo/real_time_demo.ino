/* real_time_demo.ino - Demo for displaying the real time! */
#include <ClockController.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

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

// Local Sketch Variables
long wifiTimer;
int hour, minute, second, day;

void setup() {
    Serial.begin(115200);

    // Initialize clock
    clockController.begin();
    clockController.displayLoading();

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
}

void loop() {
    clockController.loop();
    if (clockController.needsTimeUpdate()) {
      fetchTime();
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