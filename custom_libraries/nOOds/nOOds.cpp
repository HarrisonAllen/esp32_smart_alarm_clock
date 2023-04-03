/*
    nOOds.h - Library for controlling a nOOds fiber
*/

#include "nOOds.h"

nOOds::nOOds(int noodsPin) {
    _noodsPin = noodsPin;
}

void nOOds::begin() {
    pinMode(_noodsPin, OUTPUT);
}

int nOOds::setBrightness(int brightness) {
    _brightness = constrain(brightness, MIN_NOODS_BRIGHTNESS, MAX_NOODS_BRIGHTNESS);
    analogWrite(_noodsPin, _brightness);
    return _brightness;
}

int nOOds::changeBrightness(int change) {
    return setBrightness(_brightness + change);
}

int nOOds::brightnessUp(int change) {
    return changeBrightness(change);
}

int nOOds::brightnessDown(int change) {
    return changeBrightness(-change);
}

int nOOds::getBrightness() {
    return _brightness;
}
