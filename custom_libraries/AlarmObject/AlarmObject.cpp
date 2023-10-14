/*
    AlarmObject.cpp - Object for holding alarm info
*/

#include "AlarmObject.h"

AlarmObject::AlarmObject() {

}

void AlarmObject::init(Sound *sound) {
    _sound = sound;
}

void AlarmObject::setAlarmEnabled(bool enabled) {
    _alarmEnabled = enabled;
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

bool AlarmObject::checkTime(ClockController *clockController) {
    return (_currentAlarmHour == clockController->getHour()
            && _currentAlarmMinute == clockController->getMinute());
}

String AlarmObject::generateDisplayAlarm() {
    sprintf(_alarmText, "%02d:%02d", _alarmHour, _alarmMinute);
    return String(_alarmText);
}

