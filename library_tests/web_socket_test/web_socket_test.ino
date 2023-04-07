#include <ClockController.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <AlarmObject.h>
#include <Sound.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebSrv.h>
#include <Arduino_JSON.h>

// Replace with your network credentials
const char* ssid     = "Cozy Cove";
const char* password = "Prickly Mochi 1005";

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
// Create a WebSocket object
AsyncWebSocket ws("/ws");

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

    // Initialize websocket
    initWebSocket();

    // Initialize server
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SD, "/webserver/websockets/index.html", "text/html");
    });
    server.serveStatic("/", SD, "/webserver/websockets/");
    server.begin();
}

void loop() {
    clockController.loop();
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
    }
    alarmObject.setEnabled(alarmEnabledString == "true");
}

void playAlarm() {
    Serial.println("Alarm triggered!");
    sound.setSoundFile(alarmObject._alarmFilename);
    sound.setRepeating(true);
    sound.setVolume(1);
    sound.play();
    alarmPlaying = true;
}
