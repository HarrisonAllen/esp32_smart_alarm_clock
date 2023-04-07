#define T_INIT_FRAME_TIME 50

// Trellis Idle Buttons
/*
    0  1  2  3
    4  5  6  7
    8  9  10 11
    12 13 14 15
*/
#define T_BTN_IP 0
#define T_BTN_B_DOWN 1
#define T_BTN_B_AUTO 2
#define T_BTN_B_UP 3
#define T_BTN_V_DOWN 4
#define T_BTN_V_UP 5
#define T_BTN_6 6
#define T_BTN_7 7
#define T_BTN_C_HOUR 8
#define T_BTN_C_MIN_10 9
#define T_BTN_C_MIN_01 10
#define T_BTN_C_OK 11
#define T_BTN_A_HOUR 12
#define T_BTN_A_MIN_10 13
#define T_BTN_A_MIN_01 14
#define T_BTN_A_OK 15

enum TrellisState {
    ts_init,
    ts_idle,
    ts_alarm,
    ts_game
};
TrellisState trellisState;
long trellisTimer;
int trellisFrame;
char trellisMessage[5];

void trellisLoop() {
    switch (trellisState) {
        case ts_init:
            if (millis() - trellisTimer > T_INIT_FRAME_TIME) {
                if (trellisFrame < numKeys) {
                    trellis.setLED(trellisFrame);
                    trellis.writeDisplay();
                    trellisFrame += 1;
                } else if (trellisFrame < numKeys * 2) {
                    trellis.clrLED(trellisFrame - numKeys);
                    trellis.writeDisplay();
                    trellisFrame += 1;;
                } else {
                    trellisState = ts_idle;
                    trellisFrame = 0;
                }
                trellisTimer = millis();
            }
            break;
        case ts_idle:
            if (millis() - trellisTimer > 30) {
                // If a button was just pressed or released...
                if (trellis.readSwitches()) {
                    handleIdleButtons();
                }
                trellisTimer = millis();
            }
            break;
        case ts_alarm:
            if (millis() - trellisTimer > 30) {
                if (trellis.readSwitches()) {
                    snooze();
                }
                trellisTimer = millis();
            }
            break;
        case ts_game:
            break;
        default:
            break;
    }
}

void handleIdleButtons() {
    if (trellis.justPressed(T_BTN_IP)) {
        Serial.println("Displaying IP");
    } 
    else if (trellis.justPressed(T_BTN_B_DOWN)) 
    {
        Serial.println("Turning brightness down");
        sprintf(trellisMessage, "b %02d", clockController.brightnessDown());
        clockController.displayMessage(trellisMessage);
    } 
    else if (trellis.justPressed(T_BTN_B_AUTO)) 
    {
        Serial.println("Setting brightness to auto");
        clockController.setAutoBrightness(true);
        sprintf(trellisMessage, "b Au");
        clockController.displayMessage(trellisMessage);
    } 
    else if (trellis.justPressed(T_BTN_B_UP)) 
    {
        sprintf(trellisMessage, "b %02d", clockController.brightnessUp());
        clockController.displayMessage(trellisMessage);
        Serial.println("Turning brightness up");
    } 
    else if (trellis.justPressed(T_BTN_V_DOWN)) 
    {
        Serial.println("Turning volume down");
    } 
    else if (trellis.justPressed(T_BTN_V_UP)) 
    {
        Serial.println("Turning volume up");
    } 
    else if (trellis.justPressed(T_BTN_6)) 
    {
        Serial.println("Button 6 pressed");
    } 
    else if (trellis.justPressed(T_BTN_7)) 
    {
        Serial.println("Button 7 pressed");
    } 
    else if (trellis.justPressed(T_BTN_C_HOUR)) 
    {
        Serial.println("Adjusting clock hour");
    } 
    else if (trellis.justPressed(T_BTN_C_MIN_10)) 
    {
        Serial.println("Adjusting clock 10s minute");
    } 
    else if (trellis.justPressed(T_BTN_C_MIN_01)) 
    {
        Serial.println("Adjusting clock 01s minute");
    } 
    else if (trellis.justPressed(T_BTN_C_OK)) 
    {
        Serial.println("Finishing clock edit");
    } 
    else if (trellis.justPressed(T_BTN_A_HOUR)) 
    {
        Serial.println("Adjusting alarm hour");
    } 
    else if (trellis.justPressed(T_BTN_A_MIN_10)) 
    {
        Serial.println("Adjusting alarm 10s minute");
    } 
    else if (trellis.justPressed(T_BTN_A_MIN_01)) 
    {
        Serial.println("Adjusting alarm 01s minute");
    } 
    else if (trellis.justPressed(T_BTN_A_OK)) 
    {
        Serial.println("Finishing alarm edit");
    }
    // go through every button
    for (uint8_t i=0; i<numKeys; i++) {
        // if it was pressed, turn LED on
        if (trellis.justPressed(i)) {
            trellis.setLED(i);
        } 
        // if it was released, turn LED off
        if (trellis.justReleased(i)) {
            trellis.clrLED(i);
        }
    }
    // tell the trellis to set the LEDs we requested
    trellis.writeDisplay();
}

void setTrellisAlarmActive(bool alarmActive) {
    if (alarmActive) {
        trellisState = ts_alarm;
        trellis.clear();
        for (uint8_t i = 0; i < numKeys; i++) {
            trellis.setLED(i);
        }
        trellis.blinkRate(HT16K33_BLINK_2HZ);
        trellis.writeDisplay();
    } else {
        trellisState = ts_idle;
        trellis.clear();
        trellis.blinkRate(HT16K33_BLINK_OFF);
        trellis.writeDisplay();
    }
}