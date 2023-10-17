/*
    AlarmObject.h - Object for holding alarm info
*/
#ifndef AlarmObject_h
#define AlarmObject_h

#include "Arduino.h"
#include <ClockController.h>
#include <Sound.h>
#include <Arduino_JSON.h>

// initialize alarms like:
// AlarmObject alarms[numalarms];
// (in setup:)
// for (int i = 0; i < numalarms; i++) {
//     alarms[i] = AlarmObject();
// }

class AlarmObject {
    public:
        // Control Functions
        AlarmObject();
        void init(Sound *sound, ClockController *clockController);
        // - Info
        void setAlarmEnabled(bool enabled);
        void setAlarmRepeat(bool repeat);
        void setAlarmLabel(char *label);
        void setAlarmActive(bool active);
        // - Time
        void setAlarmTime(int hour, int minute);
        void setAlarmFromString(String alarmString);
        void setCurrentAlarmTime(int hour, int minute);
        // - Snooze
        void setSnoozeEnabled(bool enabled);
        void setSnoozeDuration(int duration);
        void setSnoozeLimit(int limit);
        void setSnoozesRemaining(int remaining);
        void setSnoozeActive(bool active);
        // - Sound
        void setVolumeLevel(int volume);
        void setVolumeRamp(bool ramp);
        void setSoundFile(char *soundFile);
        // - Actions
        bool checkTime();
        void checkAlarm();
        void triggerAlarm();
        void startAlarm();
        void stopAlarm();
        void snoozeAlarm();
        String generateDisplayAlarm();
        JSONVar generateJSON(JSONVar baseJSON);
        // - Helpers
        void offsetTime(int minuteOffset, int startMinute, int startHour, int *outMinute, int *outHour);
        // Variables
        Sound *_sound;
        ClockController *_clockController;
        // - Info
        bool _alarmEnabled = true; // Should alarm go off?
        bool _repeat = true; // Should alarm repeat tomorrow?
        char _label[100] = "Alarm"; // Display name
        char _alarmText[30]; // Text version of alarm time
        bool _alarmActive = false; // Is the alarm going off?
        // - Time
        int _alarmHour = 8; // Hour of the alarm
        int _alarmMinute = 0; // Minute of the alarm
        int _currentAlarmHour = _alarmHour; // Current alarm hour to check
        int _currentAlarmMinute = _alarmMinute; // Current alarm minute to check
        // - Snooze
        bool _snoozeEnabled = true; // Can we snooze?
        int _snoozeDuration = 5; // Minutes to offset the alarm by
        int _snoozeLimit = 3; // Limit to the number of snoozes
        int _snoozesRemaining = _snoozeLimit; // Current number of snoozes remaining
        bool _snoozeActive = false; // Are we currently snoozing?
        // - Sound
        int _volumeLevel = MAX_VOLUME; // Target volume level
        bool _volumeRamp = true; // Should we ramp up to the volume level?
        char _soundFile[100] = "/audio/alarm.mp3";
};


#endif