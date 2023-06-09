#include "ClockController.h"

ClockController::ClockController(Adafruit_7segment *clockDisplay, int photocellPin) {
    _clockDisplay = clockDisplay;
    _photocellPin = photocellPin;
}

void ClockController::begin() {
    pinMode(_photocellPin, INPUT);
    _timer = millis();
    _clockDisplay->begin(DISPLAY_ADDRESS);
}

void ClockController::loop() {
    _update = false;
    if (millis() - _timer > TIME_UPDATE_RATE) {
        _timer = millis();
        _second += 1;
        if (_second > 59) {
            _second = 0;
            _minute += 1;
            if (_minute > 59) {
                _minute = 0;
                _hour += 1;
                // At midnight, rollover (redundant because of time reload)
                if (_hour > 23) {
                    _hour = 0;
                }
                _needsTimeUpdate = true;
            }
        }
        _update = true;
        updateBrightness();
    }
    switch (_state) {
        case cs_init:
            // do nothing
            break;
        case cs_loading:
            if (millis() - _animTimer > LOADING_UPDATE_RATE) {
                _animFrame = (_animFrame + 1) % NUM_LOADING_SCREENS;
                writeDigitsRaw(LOADING_SCREENS_HALF_CLOCKWISE[_animFrame]);
                _animTimer = millis();
            }
            break;
        case cs_error:
            // Do nothing
            break;
        case cs_message:
            if (millis() - _animTimer > _messageDuration) {
                _state = cs_time;
            }
            break;
        case cs_time:
            if (_update) {
                displayTime();
            }
            break;
        case cs_ip:
            if (millis() - _animTimer > IP_UPDATE_RATE) {
                if (_animFrame >= 4) {
                    _state = cs_time;
                    break;
                }
                Serial.printf("IP[%d]: %d\n", _animFrame, _ip[_animFrame]);
                _clockDisplay->print(_ip[_animFrame], DEC);
                _clockDisplay->writeDisplay();
                _animTimer = millis();
                _animFrame += 1;
            }
            break;
        default:
            break;
    }
}

void ClockController::setTime(int hour, int minute, int second, int day) {
    _hour = hour;
    _minute = minute;
    _second = second;
    _day = day;
    _needsTimeUpdate = false;
    if (_state != cs_ip)
        displayTime();
}

void ClockController::ignoreTimeUpdate() {
    _needsTimeUpdate = false;
}

void ClockController::displayTime() {
    _state = cs_time;
    generateDisplayTime(true);
    Serial.println(_displayTime);
    _clockDisplay->print(_displayValue, DEC);

    _clockDisplay->writeDigitRaw(2, (_displayAm ? 0x04 : 0x08) | ((_second % 2 == 0) ? 0x02 : 0x00));
    _clockDisplay->writeDisplay();
}

void ClockController::displayLoading() {
    _state = cs_loading;
    _animFrame = 0;
    _animTimer = millis();
    writeDigitsRaw(LOADING_SCREEN);
}

void ClockController::displayIP(IPAddress ip) {
    _ip = ip;
    _state = cs_ip;
    _animFrame = 0;
    _animTimer = millis();
    drawCharArray(" IP ");
    Serial.println("IP:");
}

void ClockController::displayError(char *error) {
    _state = cs_error;
    drawCharArray(error);
}

void ClockController::displayMessage(char *message, int duration) {
    _state = cs_message;
    drawCharArray(message);
    _animTimer = millis();
    _messageDuration = duration;
}

void ClockController::writeDigitsRaw(const uint8_t *digits) {
    for (uint8_t i = 0; i < 5; i++) {
        _clockDisplay->writeDigitRaw(i < 2 ? i : i + 1, digits[i]);
    }
    _clockDisplay->writeDisplay();
}

void ClockController::drawCharArray(const char *characters) {
    for (uint8_t i = 0; i < 4; i++) {
        drawCharAtPosition(i, characters[i]);
    }
    _clockDisplay->writeDisplay();
}

void ClockController::drawCharAtPosition(uint8_t position, char character) {
    // Convert position (0-3) to display position
    if (position > 1) {
        position += 1;
    }
    Serial.printf("%d: %c -> %d = %X\n", position, character, character - ' ', SEVEN_SEG_ASCII[character - ' ']);
    _clockDisplay->writeDigitRaw(position, SEVEN_SEG_ASCII[character - ' ']);
}

void ClockController::updateBrightness() {
    if (_autoBrightness) {
        _brightness = calculateBrightness(analogRead(_photocellPin));
    }
    _clockDisplay->setBrightness(_brightness);
    _clockDisplay->writeDisplay();
}

uint8_t ClockController::calculateBrightness(int photocellReading) {
    if (photocellReading <= MIN_READING) {
        return MIN_CLOCK_BRIGHTNESS;
    } else {
        return map(photocellReading, MIN_READING, MAX_READING, MIN_CLOCK_BRIGHTNESS, MAX_CLOCK_BRIGHTNESS+1);
    }
}

int ClockController::setBrightness(int brightness) {
    _brightness = constrain(brightness, MIN_CLOCK_BRIGHTNESS, MAX_CLOCK_BRIGHTNESS);
    _autoBrightness = false;
    updateBrightness();
    return _brightness;
}

int ClockController::changeBrightness(int change) {
    return setBrightness(_brightness + change);
}

int ClockController::brightnessUp(int change) {
    return changeBrightness(change);
}

int ClockController::brightnessDown(int change) {
    return changeBrightness(-change);
}

void ClockController::setAutoBrightness(bool autoBrightness) {
    _autoBrightness = autoBrightness;
    updateBrightness();
}


ClockState ClockController::getState() {
    return _state;
}

int ClockController::getHour() {
    return _hour;
}

int ClockController::getMinute() {
    return _minute;
}

int ClockController::getSecond() {
    return _second;
}

int ClockController::getDay() {
    return _day;
}

String ClockController::generateDisplayTime(bool includeSeconds) {
    _displayAm = true;
    _displayHour = _hour;
    if (_displayHour >= 12) {
        _displayHour -= 12;
        _displayAm = false;
    }
    if (_displayHour == 0) {
        _displayHour = 12;
    }
    _displayValue = _displayHour * 100 + _minute;
    if (includeSeconds) {
        sprintf(_displayTime, "%02d:%02d:%02d %s", _displayHour, _minute, _second, _displayAm ? "AM" : "PM");
    } else {
        sprintf(_displayTime, "%02d:%02d %s", _displayHour, _minute, _displayAm ? "AM" : "PM");
    }
    return String(_displayTime);
}

bool ClockController::needsTimeUpdate() {
    return _needsTimeUpdate;
}