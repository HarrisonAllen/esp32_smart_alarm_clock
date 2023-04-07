#define TRELLIS_INIT_DURATION 50
enum TrellisState {
    ts_init,
    ts_idle,
    ts_alarm,
    ts_game
};
TrellisState trellisState;
long trellisTimer;
int trellisFrame;

void trellisLoop() {
    switch (trellisState) {
        case ts_init:
            if (millis() - trellisTimer > TRELLIS_INIT_DURATION) {
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
                    // go through every button
                    for (uint8_t i=0; i<numKeys; i++) {
                        // if it was pressed, turn LED on
                        if (trellis.justPressed(i)) {
                            Serial.print("v"); Serial.println(i);
                            trellis.setLED(i);
                            sound.setVolume(i+5);
                            sound.playOnce("/audio/pop.mp3");
                        } 
                        // if it was released, turn LED off
                        if (trellis.justReleased(i)) {
                            Serial.print("^"); Serial.println(i);
                            trellis.clrLED(i);
                        }
                    }
                    // tell the trellis to set the LEDs we requested
                    trellis.writeDisplay();
                }
                trellisTimer = millis();
            }
            break;
        case ts_alarm:
            break;
        case ts_game:
            break;
        default:
            break;
    }
}