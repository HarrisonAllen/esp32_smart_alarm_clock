/*
    nOOds.h - Library for controlling a nOOds fiber
*/
#ifndef nOOds_h
#define nOOds_h

#include "Arduino.h"

const int MIN_NOODS_BRIGHTNESS = 0;
const int MAX_NOODS_BRIGHTNESS = 255;

class nOOds {
    public:
        // Control Functions
        nOOds(int noodsPin);
        void begin();
        // Setters
        int setBrightness(int brightness);
        int changeBrightness(int change);
        int brightnessUp(int change = 1);
        int brightnessDown(int change = 1);
        // Getters
        int getBrightness();
    private:
        // Variables
        int _brightness;
        int _noodsPin;
};


#endif