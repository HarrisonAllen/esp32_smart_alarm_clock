/*
    AlarmObject.cpp - Object for holding alarm info
*/

#include "AlarmObject.h"

AlarmObject::AlarmObject() {

}

// Setters

void AlarmObject::init(Sound *sound, ClockController *clockController) {
    _sound = sound;
    _clockController = clockController;
}

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

JSONVar AlarmObject::generateJSON(JSONVar baseJSON) {
    baseJSON["alarm"]["label"] = _label; // html js_rec js_send c_rec
    baseJSON["alarm"]["alarm"]["enabled"] = _alarmEnabled ? "true" : "false"; // html js_rec js_send c_rec
    baseJSON["alarm"]["alarm"]["active"] = _alarmActive ? "true" : "false"; // html js_rec !js_send !c_rec
    baseJSON["alarm"]["alarm"]["time"] = generateDisplayAlarm(); // html js_rec js_send c_rec
    baseJSON["alarm"]["alarm"]["repeat"] = _repeat ? "true" : "false"; // html js_rec js_send c_rec

    baseJSON["alarm"]["snooze"]["enabled"] = _snoozeEnabled ? "true" : "false"; // html js_rec js_send c_rec
    baseJSON["alarm"]["snooze"]["active"] = _snoozeActive ? "true" : "false"; // html js_rec !js_send !c_rec
    baseJSON["alarm"]["snooze"]["duration"] = _snoozeDuration; // html js_rec js_send c_rec
    baseJSON["alarm"]["snooze"]["limit"] = _snoozeLimit; // html js_rec js_send c_rec
    baseJSON["alarm"]["snooze"]["remaining"] = _snoozesRemaining; // html js_rec !js_send !js_rec

    baseJSON["alarm"]["sound"]["volume"] = _volume; // html js_rec js_send c_rec
    baseJSON["alarm"]["sound"]["ramp"] = _volumeRamp ? "true" : "false"; // html js_rec js_send c_rec
    baseJSON["alarm"]["sound"]["file"] = String(_soundFile); // html js_rec js_send c_rec
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

