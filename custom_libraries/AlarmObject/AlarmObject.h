/*
    AlarmObject.h - Object for holding alarm info
*/
#ifndef AlarmObject_h
#define AlarmObject_h

#include "Arduino.h"
#include <ClockController.h>

// I think this is just a placeholder class for a database later? Or maybe it will still be used?
// Maybe on boot, load alarms from database into alarm objects
// Then, any time there's a change from webserver, whatever, update alarm objects and/or recreate all
// Can also make it so that alarms talk directly to database
// On each change to the database, reload alarms
// - aka go through each alarm in database, assign to alarms

// initialize alarms like:
// AlarmObject alarms[numalarms];
// (in setup:)
// for (int i = 0; i < numalarms; i++) {
//     alarms[i] = AlarmObject();
// }
// And in alarmobject, go ahead and peek the database to extract info

class AlarmObject {
    public:
        // Control Functions
        AlarmObject();
        void setAlarm(int hour, int minute, uint8_t days, int snoozeDuration, int snoozeLimit, bool playGame, int lightDuration, char *soundFile, bool enabled, bool deleteAfter);
        void setTime(int hour, int minute);
        void setDays(uint8_t days);
        void setDaysArray(bool *days);
        void setSnooze(int duration, int limit);
        void setPlayGame(bool play);
        void setLightDuration(int duration);
        void setAlarmSound(char *soundFile);
        void setEnabled(bool enabled);
        void setDeleteAfter(bool deleteAfter);
        void setAlarmPlaying(bool alarmPlaying);
        bool checkTime(ClockController *clockController);
        String generateDisplayAlarm();
        // Variables
        int _hour, _minute;
        uint8_t _days = 0b01111111;
        int _snoozeDuration = 5, _snoozeLimit = -1;
        bool _playGame;
        int _lightDuration = 10;
        char _alarmFilename[100] = "/audio/alarm.mp3";
        bool _enabled, _deleteAfter;
        char _displayAlarm[30];
        bool _alarmPlaying;
};


#endif