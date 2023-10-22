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
    alarmVar[alarmNum]["hidden"] = "true";
    alarmVar[alarmNum]["alarm"]["enabled"] = "true";
    alarmVar[alarmNum]["alarm"]["active"] = "true";
    alarmVar[alarmNum]["alarm"]["hour"] = 8;
    alarmVar[alarmNum]["alarm"]["minute"] = 0;
    alarmVar[alarmNum]["alarm"]["repeat"] = "true";

    alarmVar[alarmNum]["snooze"]["enabled"] = "true";
    alarmVar[alarmNum]["snooze"]["active"] = "false";
    alarmVar[alarmNum]["snooze"]["duration"] = 5;
    alarmVar[alarmNum]["snooze"]["limit"] = 3;
    alarmVar[alarmNum]["snooze"]["remaining"] = 3;

    alarmVar[alarmNum]["sound"]["volume"] = 5;
    alarmVar[alarmNum]["sound"]["ramp"] = "false";
    alarmVar[alarmNum]["sound"]["file"] = "/audio/alarm.mp3";
    return alarmVar;
}

void AlarmObject::init(Sound *sound, ClockController *clockController) {
    _sound = sound;
    _clockController = clockController;
}

// Setters

void AlarmObject::setAlarmEnabled(bool enabled) {
    Serial.printf("Alarm %s\n", (enabled ? "enabled" : "disabled"));
    _alarmEnabled = enabled;
    if (_alarmActive && enabled) {
        stopAlarm();
    }
}

void AlarmObject::setAlarmRepeat(bool repeat) {
    _repeat = repeat;
}

void AlarmObject::setAlarmLabel(String label) {
    label.toCharArray(_label, 99);
}

void AlarmObject::setAlarmActive(bool active) {
    _alarmActive = active;
}

void AlarmObject::setAlarmTime(int hour, int minute) {
    _alarmHour = hour;
    _alarmMinute = minute;
    setCurrentAlarmTime(hour, minute);
}

void AlarmObject::setAlarmFromString(String alarmString) {
    int alarmHour, alarmMinute;
    alarmHour = alarmString.substring(0, 2).toInt();
    alarmMinute = alarmString.substring(3).toInt();
    setAlarmTime(alarmHour, alarmMinute);
    setAlarmEnabled(true);
    Serial.printf("Alarm set for %d:%d\n", alarmHour, alarmMinute);
}

void AlarmObject::setCurrentAlarmTime(int hour, int minute) {
    _currentAlarmHour = hour;
    _currentAlarmMinute = minute;
}

void AlarmObject::resetAlarmTime() {
    setAlarmTime(_alarmHour, _alarmMinute);
}

void AlarmObject::setSnoozeEnabled(bool enabled) {
    _snoozeEnabled = enabled;
}

void AlarmObject::setSnoozeDuration(int duration) {
    _snoozeDuration = duration;
}

void AlarmObject::setSnoozeLimit(int limit) {
    _snoozeLimit = limit;
    setSnoozesRemaining(limit);
}

void AlarmObject::setSnoozesRemaining(int remaining) {
    _snoozesRemaining = remaining;
}

void AlarmObject::setSnoozeActive(bool active) {
    _snoozeActive = active;
}

void AlarmObject::setVolume(int volume) {
    _volume = volume;
}

void AlarmObject::setVolumeRamp(bool ramp) {
    _volumeRamp = ramp;
}

void AlarmObject::setSoundFile(String soundFile) {
    soundFile.toCharArray(_soundFile, 99);
}

// Do stuff

bool AlarmObject::checkTime() {
    return (_currentAlarmHour == _clockController->getHour()
            && _currentAlarmMinute == _clockController->getMinute());
}

void AlarmObject::checkAlarm() {
    if (checkTime()) {
        triggerAlarm();
    }
}

void AlarmObject::triggerAlarm() {
    if (_alarmEnabled && !_alarmActive) {
        Serial.println("Triggering alarm!");
        _sound->setSoundFile(_soundFile);
        _sound->setVolume(_volume);
        _sound->setRepeating(true);
        _sound->play();
        _alarmActive = true;
        _snoozeActive = false;
    }
}

void AlarmObject::stopAlarm() {
    _sound->stop();
    _alarmActive = false;
    _alarmEnabled = _repeat;
    resetAlarmTime();
    _snoozeActive = false;
    _snoozesRemaining = _snoozeLimit;
}

void AlarmObject::snoozeAlarm() {
    _sound->stop();
    _alarmActive = false;
    Serial.printf("Snoozing alarm for %d minutes\n", _snoozeDuration);
    offsetTime(_snoozeDuration, _clockController->getMinute(), _clockController->getHour(), &_currentAlarmMinute, &_currentAlarmHour);
    Serial.printf("New alarm time: %d:%d\n", _currentAlarmHour, _currentAlarmMinute);
    _snoozesRemaining--;
    _snoozeActive = true;
}

String AlarmObject::generateDisplayAlarm() {
    sprintf(_alarmText, "%02d:%02d", _alarmHour, _alarmMinute);
    return String(_alarmText);
}

void AlarmObject::parseJSON(JSONVar json) {
    _alarms = JSON.parse(json);
}

JSONVar AlarmObject::generateJSON(JSONVar baseJSON) {
    baseJSON["alarm"]["label"] = _label;
    baseJSON["alarm"]["alarm"]["enabled"] = _alarmEnabled ? "true" : "false";
    baseJSON["alarm"]["alarm"]["active"] = _alarmActive ? "true" : "false";
    baseJSON["alarm"]["alarm"]["time"] = generateDisplayAlarm();
    baseJSON["alarm"]["alarm"]["repeat"] = _repeat ? "true" : "false";

    baseJSON["alarm"]["snooze"]["enabled"] = _snoozeEnabled ? "true" : "false";
    baseJSON["alarm"]["snooze"]["active"] = _snoozeActive ? "true" : "false";
    baseJSON["alarm"]["snooze"]["duration"] = _snoozeDuration;
    baseJSON["alarm"]["snooze"]["limit"] = _snoozeLimit;
    baseJSON["alarm"]["snooze"]["remaining"] = _snoozesRemaining; // html js_rec !js_send !js_rec

    baseJSON["alarm"]["sound"]["volume"] = _volume;
    baseJSON["alarm"]["sound"]["ramp"] = _volumeRamp ? "true" : "false";
    baseJSON["alarm"]["sound"]["file"] = String(_soundFile);
    return baseJSON;
}

void AlarmObject::offsetTime(int minuteOffset, int startMinute, int startHour, int *outMinute, int *outHour) {
    *outMinute = startMinute + minuteOffset;
    *outHour = startHour;
    if (*outMinute >= 60) {
        *outMinute -= 60;
        *outHour += 1;
        if (*outHour > 23) {
            *outHour = 0;
        }
    }
}

