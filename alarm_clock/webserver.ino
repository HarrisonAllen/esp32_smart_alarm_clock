void notFound(AsyncWebServerRequest *request) {
    Serial.println("Uh oh, 404 error!");
    request->send(SD, "/webserver/404.html", "text/html");
}

String getData() {
    jsonData["currentTime"] = clockController.generateDisplayTime(true);
    jsonData = alarmObject.generateJSON(jsonData);

    String jsonString = JSON.stringify(jsonData);
    Serial.println(jsonString);
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
        if (message[0] == 'l') {
            Serial.println("New alarm label: " + message.substring(1));
            alarmObject.setAlarmLabel(message.substring(1));
            notifyClients(getData());
        }
        if (message[0] == 'e') {
            Serial.println("Alarm enabled: " + message.substring(1));
            alarmObject.setAlarmEnabled(message.substring(1) == "true");
            notifyClients(getData());
        }
        if (message[0] == 't') {
            Serial.println("New alarm time: " + message.substring(1));
            alarmObject.setAlarmFromString(message.substring(1));
            notifyClients(getData());
        }
        if (message[0] == 'r') {
            Serial.println("Alarm repeats: " + message.substring(1));
            alarmObject.setAlarmRepeat(message.substring(1) == "true");
            notifyClients(getData());
        }
        if (message[0] == 'E') {
            Serial.println("Snooze enabled: " + message.substring(1));
            alarmObject.setSnoozeEnabled(message.substring(1) == "true");
            notifyClients(getData());
        }
        if (message[0] == 'd') {
            Serial.println("Snooze duration: " + message.substring(1));
            alarmObject.setSnoozeDuration(message.substring(1).toInt());
            notifyClients(getData());
        }
        if (message[0] == 'L') {
            Serial.println("Snooze limit: " + message.substring(1));
            alarmObject.setSnoozeLimit(message.substring(1).toInt());
            notifyClients(getData());
        }
        if (message[0] == 'v') {
            Serial.println("Volume: " + message.substring(1));
            alarmObject.setVolume(message.substring(1).toInt());
            notifyClients(getData());
        }
        if (message[0] == 'R') {
            Serial.println("Volume ramp enabled: " + message.substring(1));
            alarmObject.setVolumeRamp(message.substring(1) == "true");
            notifyClients(getData());
        }
        if (message[0] == 'f') {
            Serial.println("New sound file: " + message.substring(1));
            alarmObject.setSoundFile(message.substring(1));
            notifyClients(getData());
        }
        if (message[0] == 's') {
            Serial.println("Snooze received");
            alarmObject.snoozeAlarm();
            notifyClients(getData());
        }
        if (message[0] == 'S') {
            Serial.println("Stop received");
            alarmObject.stopAlarm();
            notifyClients(getData());
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

void setupWebPages() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SD, "/webserver/index.html", "text/html");
  });
  server.on("/alarm", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SD, "/webserver/alarm.html", "text/html");
  });
  server.serveStatic("/", SD, "/webserver/static/");
  server.onNotFound(notFound);
}