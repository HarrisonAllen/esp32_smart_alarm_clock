/*
    AlarmObject.cpp - Object for holding alarm info
*/

#include "AlarmObject.h"

AlarmObject::AlarmObject() {

}

JSONVar AlarmObject::createAlarm(JSONVar alarmVar, int alarmNum) {
    alarmVar[alarmNum]["alarm"]["enabled"] = true;
    alarmVar[alarmNum]["alarm"]["active"] = false;
    alarmVar[alarmNum]["alarm"]["hour"] = 8;
    alarmVar[alarmNum]["alarm"]["minute"] = 0;
    alarmVar[alarmNum]["alarm"]["currentHour"] = (int)alarmVar[alarmNum]["alarm"]["hour"];
    alarmVar[alarmNum]["alarm"]["currentMinute"] = (int)alarmVar[alarmNum]["alarm"]["minute"];
    alarmVar[alarmNum]["alarm"]["repeat"] = true;

    alarmVar[alarmNum]["snooze"]["enabled"] = true;
    alarmVar[alarmNum]["snooze"]["active"] = false;
    alarmVar[alarmNum]["snooze"]["duration"] = 5;
    alarmVar[alarmNum]["snooze"]["limit"] = 3;
    alarmVar[alarmNum]["snooze"]["remaining"] = 3;

    alarmVar[alarmNum]["sound"]["volume"] = MAX_VOLUME;
    alarmVar[alarmNum]["sound"]["file"] = "/audio/alarm.mp3";
    return alarmVar;
}

void AlarmObject::init(int numAlarms, Sound *sound, ClockController *clockController, NTPClient *timeClient) {
    _sound = sound;
    _clockController = clockController;
    _timeClient = timeClient;
    _numAlarms = numAlarms;

    if (SD.exists("/config.json")) {
        File configFile = SD.open("/config.json");
        if (configFile) {
            String configString = "";
            while (configFile.available()) {
                configString += (char)configFile.read();
            }
            configFile.close();
            Serial.println("Successfully loaded config from file");
            parseString(configString);
        } else {
            Serial.println("Failed to load config from file");
        }
    } else {
        _alarms[0]["timeOffset"] = 0;
        for (int i = 0; i < _numAlarms; i++) {
            _alarms = createAlarm(_alarms, i); 
        }
        Serial.println("No config found. Created new alarms");
    }
}

// Setters

void AlarmObject::resetAlarmTime(int alarmNum) {
    _alarms[alarmNum]["alarm"]["currentHour"] = (int)_alarms[alarmNum]["alarm"]["hour"];
    _alarms[alarmNum]["alarm"]["currentMinute"] = (int)_alarms[alarmNum]["alarm"]["minute"];
}

// Do stuff

bool AlarmObject::checkTime(int alarmNum) {
    return ((int)_alarms[alarmNum]["alarm"]["currentHour"] == _clockController->getHour()
            && (int)_alarms[alarmNum]["alarm"]["currentMinute"] == _clockController->getMinute());
}

bool AlarmObject::checkAlarms() {
    bool anyAlarmTriggered = false;
    for (int i = 0; i < _numAlarms; i++) {
        if (checkTime(i)) {
            anyAlarmTriggered = (anyAlarmTriggered || triggerAlarm(i));
        }
    }
    return false;
}

bool AlarmObject::triggerAlarm(int alarmNum) {
    if ((bool)_alarms[alarmNum]["alarm"]["enabled"] && !(bool)_alarms[alarmNum]["alarm"]["active"]) {
        Serial.println("Triggering alarm!");
        strcpy(_soundFileBuffer, (const char*)_alarms[alarmNum]["sound"]["file"]);
        // ().toCharArray(_soundFileBuffer, 99);
        Serial.println(_soundFileBuffer);
        _sound->setSoundFile(_soundFileBuffer);
        _sound->setVolume((int)_alarms[alarmNum]["sound"]["volume"]);
        _sound->setRepeating(true);
        _sound->play();
        _alarms[alarmNum]["alarm"]["active"] = true;
        _alarms[alarmNum]["snooze"]["active"] = false;
        return true;
    }
    return false;
}

void AlarmObject::stopAlarms() {
    _sound->stop();
    for (int i = 0; i < _numAlarms; i++) {
        if ((bool)_alarms[i]["alarm"]["active"] || (bool)_alarms[i]["snooze"]["active"]) {
            _alarms[i]["alarm"]["active"] = false;
            _alarms[i]["alarm"]["enabled"] = (bool)_alarms[i]["alarm"]["repeat"];
            resetAlarmTime(i);
            _alarms[i]["snooze"]["active"] = false;
            _alarms[i]["snooze"]["remaining"] = (int)_alarms[i]["snooze"]["limit"];
        }
    }
}

void AlarmObject::snoozeAlarms() {
    bool anyAlarmsStayOn = false;
    for (int i = 0; i < _numAlarms; i++) {
        if ((bool)_alarms[i]["alarm"]["active"])
        {
            if ((bool)_alarms[i]["snooze"]["enabled"]) {
                _alarms[i]["alarm"]["active"] = false;
                Serial.printf("Snoozing alarm for %d minutes\n", (int)_alarms[i]["snooze"]["duration"]);
                offsetAlarm(i);
                Serial.printf("New alarm time: %d:%d\n", (int)_alarms[i]["alarm"]["currentHour"], (int)_alarms[i]["alarm"]["currentMinute"]);
                _alarms[i]["snooze"]["remaining"] = (int)_alarms[i]["snooze"]["remaining"] - 1;
                _alarms[i]["snooze"]["active"] = true;
            } else {
                anyAlarmsStayOn = true;
            }
        }
    }
    if (!anyAlarmsStayOn) _sound->stop();
}

void AlarmObject::parseString(String stringToParse) {
    int oldTimeOffset = (int)_alarms[0]["timeOffset"];
    _alarms = JSON.parse(stringToParse);
    int newTimeOffset = (int)_alarms[0]["timeOffset"];
    if (oldTimeOffset != newTimeOffset) {
        _timeOffsetChanged = true;
        _timeClient->setTimeOffset(newTimeOffset);
    }
    
    if (SD.exists("/config.json")) {
        Serial.println("Removed old config");
        SD.remove("/config.json");
    }
    
    File configFile = SD.open("/config.json", FILE_WRITE);
    configFile.close();

    configFile = SD.open("/config.json", FILE_WRITE);
    if (configFile) {
        configFile.print(stringToParse);
        configFile.close();
        Serial.println("Successfully saved config to file");
    } else {
        Serial.println("Failed to save config to file");
    }
}

void AlarmObject::offsetAlarm(int alarmNum) {
    _alarms[alarmNum]["alarm"]["currentMinute"] = _clockController->getMinute() + (int)_alarms[alarmNum]["snooze"]["duration"];
    _alarms[alarmNum]["alarm"]["currentHour"] = _clockController->getHour();
    if ((int)_alarms[alarmNum]["alarm"]["currentMinute"] >= 60) {
        _alarms[alarmNum]["alarm"]["currentMinute"] = (int)_alarms[alarmNum]["alarm"]["currentMinute"] - 60;
        _alarms[alarmNum]["alarm"]["currentHour"] = (int)_alarms[alarmNum]["alarm"]["currentHour"] + 1;
        if ((int)_alarms[alarmNum]["alarm"]["currentHour"] > 23) {
            _alarms[alarmNum]["alarm"]["currentHour"] = 0;
        }
    }
}
