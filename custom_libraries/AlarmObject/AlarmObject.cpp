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
    // Turn off alarm?
}

void AlarmObject::setAlarmRepeat(bool repeat) {
    _repeat = repeat;
}

void AlarmObject::setAlarmLabel(char *label) {
    strcpy(_label, label);
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

void AlarmObject::setSnoozeEnabled(bool enabled) {
    _snoozeEnabled = enabled;
}

void AlarmObject::setSnoozeDuration(int duration) {
    _snoozeDuration = duration;
}

void AlarmObject::setSnoozeLimit(int limit) {
    _snoozeLimit = limit;
}

void AlarmObject::setSnoozesRemaining(int remaining) {
    _snoozesRemaining = remaining;
}

void AlarmObject::setSnoozeActive(bool active) {
    _snoozeActive = active;
}

void AlarmObject::setVolumeLevel(int volume) {
    _volumeLevel = volume;
}

void AlarmObject::setVolumeRamp(bool ramp) {
    _volumeRamp = ramp;
}

void AlarmObject::setSoundFile(char *soundFile) {
    strcpy(_soundFile, soundFile);
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
        startAlarm();
    }
}

void AlarmObject::startAlarm() {
    if (_alarmEnabled && !_alarmActive) {
        _sound->setSoundFile(_soundFile);
        _sound->setRepeating(true);
        _sound->play();
        _alarmActive = true;
    }
}

void AlarmObject::stopAlarm() {
    _sound->stop();
    _alarmActive = false;
}

void AlarmObject::snoozeAlarm() {
    if (_alarmActive) {
        stopAlarm();
        Serial.printf("Snoozing alarm for %d minutes\n");
        offsetTime(_snoozeDuration, _clockController->getMinute(), _clockController->getHour(), &_currentAlarmMinute, &_currentAlarmHour);
        Serial.printf("New alarm time: %d:%d\n", _currentAlarmHour, _currentAlarmMinute);
    }
}

String AlarmObject::generateDisplayAlarm() {
    sprintf(_alarmText, "%02d:%02d", _alarmHour, _alarmMinute);
    return String(_alarmText);
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

