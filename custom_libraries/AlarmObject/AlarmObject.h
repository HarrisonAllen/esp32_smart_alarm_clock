/*
    AlarmObject.h - Object for holding alarm info
*/
#ifndef AlarmObject_h
#define AlarmObject_h

#include "Arduino.h"
#include <ClockController.h>
#include <Sound.h>
#include <Arduino_JSON.h>

#define NUM_ALARMS 1

class AlarmObject {
    public:
        // Control Functions
        AlarmObject();
        JSONVar createAlarm(JSONVar alarmVar, int index);
        void init(Sound *sound, ClockController *clockController);
        // - Actions
        void resetAlarmTime();
        bool checkTime();
        bool checkAlarm();
        bool triggerAlarm();
        void stopAlarm();
        void snoozeAlarm();
        void parseString(String stringToParse);
        // - Helpers
        void offsetAlarm();
        // Variables
        Sound *_sound;
        ClockController *_clockController;
        JSONVar _alarms;
        char _soundFileBuffer[100];
};


#endif