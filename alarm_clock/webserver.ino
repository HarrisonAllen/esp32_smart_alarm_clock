void notFound(AsyncWebServerRequest *request) {
    Serial.println("Uh oh, 404 error!");
    request->send(SD, "/webserver/404.html", "text/html");
}

String getData() {
    String jsonString = JSON.stringify(alarmObject._alarms);
    Serial.print("Data to send: ");
    Serial.println(jsonString);
    return jsonString;
}

void notifyClients(String dataString) {
    sound.pause();
    ws.textAll(dataString);
    sound.resume();
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
    sound.pause();
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        data[len] = 0;
        message = (char*)data;
        Serial.println("Received: " + message);
        if (message == "getData") {
            Serial.println("Just getting data");
        } else if (message == "snooze") {
            alarmObject.snoozeAlarms();
        } else if (message == "stop") {
            alarmObject.stopAlarms();
        } else {
            alarmObject.parseString(message);
        }
        notifyClients(getData());
    }
    sound.resume();
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
    sound.pause();
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SD, "/webserver/index.html", "text/html");
    });
    server.on("/alarm", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SD, "/webserver/alarm.html", "text/html");
    });
    server.on("/snooze", HTTP_GET, [](AsyncWebServerRequest *request) {
        alarmObject.snoozeAlarms();
        request->send(200, "text/plain", "Snoozed");
    });
    server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request) {
        alarmObject.stopAlarms();
        request->send(200, "text/plain", "Stopped");
    });
    server.serveStatic("/", SD, "/webserver/static/");
    server.onNotFound(notFound);
    sound.resume();
}