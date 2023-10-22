void notFound(AsyncWebServerRequest *request) {
    Serial.println("Uh oh, 404 error!");
    request->send(SD, "/webserver/404.html", "text/html");
}

String getData() {
    String jsonString = JSON.stringify(alarmObject._alarms);
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
        alarmObject.parseJSON(message);
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