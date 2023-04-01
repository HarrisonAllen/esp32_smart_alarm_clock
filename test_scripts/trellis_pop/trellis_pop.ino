#include <Wire.h>
#include "Adafruit_Trellis.h"
#include "Arduino.h"
#include "Audio.h"
#include "SD.h"
#include "FS.h"

// I2S Connections
#define I2S_DOUT      27
#define I2S_BCLK      14
#define I2S_LRC       12

// Create Audio object
Audio audio;

Adafruit_Trellis matrix0 = Adafruit_Trellis();

// Just one
Adafruit_TrellisSet trellis =  Adafruit_TrellisSet(&matrix0);

#define numKeys 16

long trellisTimer;

void setup() {
  Serial.begin(115200);
    
  // Start microSD Card
  if(!SD.begin())
  {
    Serial.println("Error accessing microSD card!");
    while(true); 
  }
  
  // Setup I2S 
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);

  trellis.begin(0x71);  // only one

  // light up all the LEDs in order
  for (uint8_t i=0; i<numKeys; i++) {
    trellis.setLED(i);
    trellis.writeDisplay();    
    delay(50);
  }
  // then turn them off
  for (uint8_t i=0; i<numKeys; i++) {
    trellis.clrLED(i);
    trellis.writeDisplay();    
    delay(50);
  }

  trellisTimer = millis();
}


void loop() {
  if (millis() - trellisTimer > 30) {
    // If a button was just pressed or released...
    if (trellis.readSwitches()) {
      // go through every button
      for (uint8_t i=0; i<numKeys; i++) {
        // if it was pressed, turn it on
        if (trellis.justPressed(i)) {
          Serial.print("v"); Serial.println(i);
          trellis.setLED(i);
          audio.setVolume(i+5);
          audio.connecttoFS(SD,"/audio/pop.mp3");
        } 
        // if it was released, turn it off
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
  audio.loop();
}

