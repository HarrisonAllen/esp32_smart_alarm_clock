/*
    AlarmObject.cpp - Object for holding alarm info
*/

#include "AlarmObject.h"

AlarmObject::AlarmObject() {
    for (int i = 0; i < NUM_ALARMS; i++) {
       _alarms = createAlarm(_alarms, i); 
    }
}

JSONVar AlarmObject::createAlarm(JSONVar alarmVar, int alarmNum) {
    alarmVar[alarmNum]["label"] = "Alarm " + (String)(alarmNum + 1);
    alarmVar[alarmNum]["hidden"] = true;
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
    alarmVar[alarmNum]["sound"]["ramp"] = false;
    alarmVar[alarmNum]["sound"]["file"] = "/audio/alarm.mp3";
    return alarmVar;
}

void AlarmObject::init(Sound *sound, ClockController *clockController) {
    _sound = sound;
    _clockController = clockController;
}

// Setters

void AlarmObject::resetAlarmTime() {
    _alarms[0]["alarm"]["currentHour"] = (int)_alarms[0]["alarm"]["hour"];
    _alarms[0]["alarm"]["currentMinute"] = (int)_alarms[0]["alarm"]["minute"];
}

// Do stuff

bool AlarmObject::checkTime() {
    return ((int)_alarms[0]["alarm"]["currentHour"] == _clockController->getHour()
            && (int)_alarms[0]["alarm"]["currentMinute"] == _clockController->getMinute());
}

bool AlarmObject::checkAlarm() {
    if (checkTime()) {
        return triggerAlarm();
    }
    return false;
}

bool AlarmObject::triggerAlarm() {
    if ((bool)_alarms[0]["alarm"]["enabled"] && !(bool)_alarms[0]["alarm"]["active"]) {
        Serial.println("Triggering alarm!");
        strcpy(_soundFileBuffer, (const char*)_alarms[0]["sound"]["file"]);
        // ().toCharArray(_soundFileBuffer, 99);
        Serial.println(_soundFileBuffer);
        _sound->setSoundFile(_soundFileBuffer);
        _sound->setVolume((int)_alarms[0]["sound"]["volume"]);
        _sound->setRepeating(true);
        _sound->play();
        _alarms[0]["alarm"]["active"] = true;
        _alarms[0]["snooze"]["active"] = false;
        return true;
    }
    return false;
}

void AlarmObject::stopAlarm() {
    _sound->stop();
    _alarms[0]["alarm"]["active"] = false;
    _alarms[0]["alarm"]["enabled"] = (bool)_alarms[0]["alarm"]["repeat"];
    resetAlarmTime();
    _alarms[0]["snooze"]["active"] = false;
    _alarms[0]["snooze"]["remaining"] = (int)_alarms[0]["snooze"]["limit"];
}

void AlarmObject::snoozeAlarm() {
    _sound->stop();
    _alarms[0]["alarm"]["active"] = false;
    Serial.printf("Snoozing alarm for %d minutes\n", (int)_alarms[0]["snooze"]["duration"]);
    offsetAlarm();
    Serial.printf("New alarm time: %d:%d\n", (int)_alarms[0]["alarm"]["currentHour"], (int)_alarms[0]["alarm"]["currentMinute"]);
    _alarms[0]["snooze"]["remaining"] = (int)_alarms[0]["snooze"]["remaining"] - 1;
    _alarms[0]["snooze"]["active"] = true;
}

void AlarmObject::parseString(String stringToParse) {
    _alarms = JSON.parse(stringToParse);
    
    if ((bool)_alarms[0]["alarm"]["active"] && !(bool)_alarms[0]["alarm"]["enabled"]) {
        stopAlarm();
    }
    resetAlarmTime();
}

void AlarmObject::offsetAlarm() {
    _alarms[0]["alarm"]["currentMinute"] = _clockController->getMinute() + (int)_alarms[0]["snooze"]["duration"];
    _alarms[0]["alarm"]["currentHour"] = _clockController->getHour();
    if ((int)_alarms[0]["alarm"]["currentMinute"] >= 60) {
        _alarms[0]["alarm"]["currentMinute"] = (int)_alarms[0]["alarm"]["currentMinute"] - 60;
        _alarms[0]["alarm"]["currentHour"] = (int)_alarms[0]["alarm"]["currentHour"] + 1;
        if ((int)_alarms[0]["alarm"]["currentHour"] > 23) {
            _alarms[0]["alarm"]["currentHour"] = 0;
        }
    }
}
