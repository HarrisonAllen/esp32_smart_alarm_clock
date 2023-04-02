
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include "7-Segment-ASCII_HEX.h"
#include "Adafruit_Trellis.h"
#include "Audio.h"
#include "SD.h"
#include "FS.h"

// Wifi credentials
const char* ssid     = "Cozy Cove";
const char* password = "Prickly Mochi 1005";

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// I2C address of the 7-segment display
#define DISPLAY_ADDRESS   0x70
Adafruit_7segment clockDisplay = Adafruit_7segment();
const uint8_t LOADING_SCREEN[] = {0b00111001, 0b00001001, 0b00001001, 0b00001111};

// Time data info
int hour, minute, second, day;
int displayHour;
int displayValue;
const String DAYS_OF_THE_WEEK[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
long clockTimer;
uint8_t clockBrightness = 0;

// Photocell
const int PHOTOCELL_PIN = 36;     // the cell and 10K pulldown are connected to pin 36
const int MIN_READING = 800; // Where I would say a room is "dark"
const int MAX_READING = 4096; // 2^12 - 1
const int MIN_CLOCK_BRIGHTNESS = 0;
const int MAX_CLOCK_BRIGHTNESS = 15;

// nOOds fiber
const int NOODS_PIN = 2;
const int MIN_NOODS_BRIGHTNESS = 0;
const int MAX_NOODS_BRIGHTNESS = 255;
const int NOODS_UPDATE_RATE = 100;
long noodsTimer;

// I2S Connections
#define I2S_DOUT      27
#define I2S_BCLK      14
#define I2S_LRC       12

// Audio object
Audio audio;

// Trellis
#define numKeys 16
Adafruit_Trellis matrix0 = Adafruit_Trellis();
Adafruit_TrellisSet trellis =  Adafruit_TrellisSet(&matrix0);
long trellisTimer;


void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  
  // Setup the display.
  clockDisplay.begin(DISPLAY_ADDRESS);
  displayLoading();
  
  // Start microSD Card
  if(!SD.begin())
  {
    Serial.println("Error accessing microSD card!");
    while(true); 
  }
  
  // Connect to wifi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Stop blinking after wifi connected
  clockDisplay.blinkRate(0);
  clockDisplay.writeDisplay();

  // Display local IP address
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  displayIP();
  
  // Setup I2S 
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);

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
  clockTimer = millis();
  fetchTime();
  displayTime();

  // nOOds
  noodsTimer = millis();
}

void loop() {
  timeLoop();
  noodsLoop();
  trellisLoop();
  audio.loop();
}

void timeLoop() {
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

void noodsLoop() {
  if (millis() - noodsTimer > NOODS_UPDATE_RATE) {
    analogWrite(NOODS_PIN, calculateNoodsBrightness(analogRead(PHOTOCELL_PIN)));
    noodsTimer = millis();
  }  
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
          audio.setVolume(i+5);
          audio.connecttoFS(SD,"/audio/pop.mp3");
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

void displayIP() {
  IPAddress ip = WiFi.localIP();
  Serial.println("It's ip time");
  drawCharArray(" IP ");
  delay(2000);
  for (uint8_t i = 0; i < 4; i++) {
    Serial.println(ip[i]);
    clockDisplay.print(ip[i], DEC);
    clockDisplay.writeDisplay();
    delay(2000);
  }
  Serial.println("-----");
}

void displayLoading() {
  clockDisplay.blinkRate(1);
  writeDigitsRaw(LOADING_SCREEN);
}

void writeDigitsRaw(const uint8_t *digits) {
  for (uint8_t i = 0; i < 5; i++) {
    clockDisplay.writeDigitRaw(i < 2 ? i : i + 1, digits[i]);
  }
  clockDisplay.writeDisplay();
}

void drawCharArray(const char *characters) {
  for (uint8_t i = 0; i < 4; i++) {
    drawCharacterAtPosition(i, characters[i]);
  }
  clockDisplay.writeDisplay();
}

void drawCharacterAtPosition(uint8_t position, char character) {
  // Convert position (0-3) to display position
  if (position > 1) {
    position += 1;
  }
  Serial.printf("%d: %c -> %d = %X\n", position, character, character - ' ', SevenSegmentASCII[character - ' ']);
  clockDisplay.writeDigitRaw(position, SevenSegmentASCII[character - ' ']);
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
  clockBrightness = calculateClockBrightness(analogRead(PHOTOCELL_PIN));
  clockDisplay.setBrightness(clockBrightness);
  clockDisplay.writeDisplay();
}

uint8_t calculateClockBrightness(int photocellReading) {
  if (photocellReading <= MIN_READING) {
    return MIN_CLOCK_BRIGHTNESS;
  } else {
    return map(photocellReading, MIN_READING, MAX_READING, MIN_CLOCK_BRIGHTNESS, MAX_CLOCK_BRIGHTNESS+1);
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
  } else {
    Serial.println("Failed to fetch time");
  }
}
