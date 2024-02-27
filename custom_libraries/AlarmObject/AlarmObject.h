/*
    AlarmObject.h - Object for holding alarm info
*/
#ifndef AlarmObject_h
#define AlarmObject_h

#include "Arduino.h"
#include <ClockController.h>
#include <Sound.h>
#include <NTPClient.h>
#include <Arduino_JSON.h>

class AlarmObject {
    public:
        // Control Functions
        AlarmObject(int num_alarms);
        JSONVar createAlarm(JSONVar alarmVar, int index);
        void init(Sound *sound, ClockController *clockController, NTPClient *timeClient);
        // - Actions
        void resetAlarmTime(int alarmNum);
        bool checkTime(int alarmNum);
        bool checkAlarms();
        bool triggerAlarm(int alarmNum);
        void stopAlarms();
        void snoozeAlarms();
        void parseString(String stringToParse);
        void offsetAlarm(int alarmNum);
        // Variables
        int _num_alarms;
        Sound *_sound;
        ClockController *_clockController;
        NTPClient *_timeClient;
        JSONVar _alarms;
        char _soundFileBuffer[100];
        bool _timeOffsetChanged;
};


#endif