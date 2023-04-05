/*
    AlarmObject.cpp - Object for holding alarm info
*/

#include "AlarmObject.h"

AlarmObject::AlarmObject() {
    
}

void AlarmObject::setAlarm(int hour, int minute, uint8_t days, int snoozeDuration, int snoozeLimit, bool playGame, int lightDuration, char *soundFile, bool enabled, bool deleteAfter) {
    setTime(hour, minute);
    setDays(days);
    setSnooze(snoozeDuration, snoozeLimit);
    setPlayGame(playGame);
    setLightDuration(lightDuration);
    setAlarmSound(soundFile);
    setEnabled(enabled);
    setDeleteAfter(deleteAfter);
}

void AlarmObject::setTime(int hour, int minute) {
    _hour = hour;
    _minute = minute;
}

void AlarmObject::setDays(uint8_t days) {
    _days = days;
}

void AlarmObject::setDaysArray(bool *days) {
    _days = 0;
    for (int i = 0; i < 7; i++) {
        if (days[i]) {
            _days |= (1 << i);
        }
    }
}

void AlarmObject::setSnooze(int duration, int limit) {
    _snoozeDuration = duration;
    _snoozeLimit = limit;
}

void AlarmObject::setPlayGame(bool play) {
    _playGame = play;
}

void AlarmObject::setLightDuration(int duration) {
    _lightDuration = duration;
}

void AlarmObject::setAlarmSound(char *soundFile) {
    strcpy(_alarmFilename, soundFile);
}

bool AlarmObject::checkTime(ClockController *clockController) {
    return (_hour == clockController->getHour()
            && _minute == clockController->getMinute()
            && (_days & (1 << clockController->getDay())));
}

void AlarmObject::setEnabled(bool enabled) {
    _enabled = enabled;
}

void AlarmObject::setDeleteAfter(bool deleteAfter) {
    _deleteAfter = deleteAfter;
}

