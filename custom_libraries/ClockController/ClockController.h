/*
    ClockController.h - Library for the clock and alarm system
*/
#ifndef ClockController_h
#define ClockController_h

#include "Arduino.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include "ClockGraphics.h"

#define DISPLAY_ADDRESS 0x70
#define MAX_TIME_TRIES 10
#define TIME_UPDATE_RATE 1000
#define LOADING_UPDATE_RATE 80
#define IP_UPDATE_RATE 1000

// Photocell
const int MIN_READING = 800; // Where I would say a room is "dark"
const int MAX_READING = 4096; // 2^12 - 1
const int MIN_CLOCK_BRIGHTNESS = 0;
const int MAX_CLOCK_BRIGHTNESS = 15;

enum ClockState {
    cs_init,
    cs_loading,
    cs_error,
    cs_message,
    cs_time,
    cs_ip
};

class ClockController {
    public:

        ClockController(Adafruit_7segment *clockDisplay, int photocellPin);
        void begin();
        void loop();
        void setTime(int hour, int minute, int second, int day);
        void ignoreTimeUpdate();
        void displayTime();
        void displayLoading();
        void displayIP(IPAddress ip);
        void displayError(char *error);
        void displayMessage(char *message, int duration=1000);
        void writeDigitsRaw(const uint8_t *digits);
        void drawCharArray(const char *characters);
        void drawCharAtPosition(uint8_t position, char character);
        void updateBrightness();
        uint8_t calculateBrightness(int photocellReading);
        ClockState getState();
        int getHour();
        int getMinute();
        int getSecond();
        int getDay();
        String generateDisplayTime(bool includeSeconds=false);
        bool needsTimeUpdate();
    private:

        Adafruit_7segment *_clockDisplay;
        int _hour, _minute, _second, _day;
        int _displayHour, _displayValue, _displayAm;
        long _timer, _animTimer;
        int _photocellPin;
        uint8_t _brightness;
        ClockState _state;
        bool _update;
        int _animFrame;
        IPAddress _ip;
        bool _needsTimeUpdate = true;
        int _messageDuration;
        char _displayTime[30];
};

#endif