/*********
  Thanks to Rui Santos for this script
  Complete project details at https://randomnerdtutorials.com
  Based on the NTP Client library example
*********/
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Replace with your network credentials
const char* ssid     = "Cozy Cove";
const char* password = "Prickly Mochi 1005";

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Time data info
int cur_hour, cur_min, cur_sec, cur_day;
char time_buffer[30];
const String DAYS_OF_THE_WEEK[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

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

// Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(-14400); // GMT -4
}

void loop() {
  timeClient.update();
  cur_hour = timeClient.getHours();
  cur_min = timeClient.getMinutes();
  cur_sec = timeClient.getSeconds();
  cur_day = timeClient.getDay();
  String am_pm = "AM";
  if (cur_hour > 12) {
    cur_hour -= 12;
    am_pm = "PM";
  } else if (cur_hour == 0) {
    cur_hour = 12;
  }

  sprintf(time_buffer, "%02d:%02d:%02d %s - %s", cur_hour, cur_min, cur_sec, am_pm, DAYS_OF_THE_WEEK[cur_day]);
  

  Serial.println(time_buffer);

  delay(1000);
}