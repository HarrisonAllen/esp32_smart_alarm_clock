/*********
  Thanks to Rui Santos for this script
  Complete project details at https://randomnerdtutorials.com
  Based on the NTP Client library example
*********/
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"\

// Replace with your network credentials
const char* ssid     = "Cozy Cove";
const char* password = "Prickly Mochi 1005";

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// I2C address of the 7-segment display
#define DISPLAY_ADDRESS   0x70
Adafruit_7segment clockDisplay = Adafruit_7segment();

// Time data info
int hour, minute, second, day;
int displayHour;
int displayValue;
const String DAYS_OF_THE_WEEK[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
long clockTimer;
uint8_t brightness = 0;

// Photocell
int photocellPin = 36;     // the cell and 10K pulldown are connected to pin 36
const int MIN_READING = 800; // Where I would say a room is "dark"
const int MAX_READING = 4096; // 2^12 - 1
const int MIN_BRIGHTNESS = 0;
const int MAX_BRIGHTNESS = 15;

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  // Setup the display.
  clockDisplay.begin(DISPLAY_ADDRESS);
  clockTimer = millis();

  // Initialize a NTPClient to get time
  timeClient.begin();
  timeClient.setTimeOffset(-14400); // GMT -4, hours -> seconds
  clockTimer = millis();
  fetchTime();
  displayTime();
}

void loop() {
  if (millis() - clockTimer > 1000) {
    clockTimer = millis();
    second += 1;
    if (second > 59) {
      second = 0;
      minute += 1;
      if (minute > 59) {
        minute = 0;
        hour += 1;
        // At midnight, rollover (redundant because of time reload)
        if (hour > 23) {
          hour = 0;
        }
        // Reload the time at the top of every hour
        fetchTime();
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

  Serial.printf("%02d:%02d:%02d %s - %s\n", displayHour, minute, second, am ? "AM" : "PM", DAYS_OF_THE_WEEK[day]);
  clockDisplay.print(displayValue, DEC);

  clockDisplay.writeDigitRaw(2, (am ? 0x04 : 0x08) | ((second % 2 == 0) ? 0x02 : 0x00));
  brightness = calculateBrightness(analogRead(photocellPin));
  clockDisplay.setBrightness(brightness);
  clockDisplay.writeDisplay();
}

uint8_t calculateBrightness(int photocellReading) {
  if (photocellReading <= MIN_READING) {
    return MIN_BRIGHTNESS;
  } else {
    return map(photocellReading, MIN_READING, MAX_READING, MIN_BRIGHTNESS, MAX_BRIGHTNESS+1);
  }
}

void fetchTime() {
  if (timeClient.update()) {
    hour = timeClient.getHours();
    minute = timeClient.getMinutes();
    second = timeClient.getSeconds();
    day = timeClient.getDay();
    Serial.println("Fetched time: " + timeClient.getFormattedTime());
  } else {
    Serial.println("Failed to fetch time");
  }
}