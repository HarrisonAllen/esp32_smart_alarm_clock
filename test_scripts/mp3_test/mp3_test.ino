
/*
  ESP32 SD I2S Music Player
  esp32-i2s-sd-player.ino
  Plays MP3 file from microSD card
  Uses MAX98357 I2S Amplifier Module
  Uses ESP32-audioI2S Library - https://github.com/schreibfaul1/ESP32-audioI2S
  * 
  DroneBot Workshop 2022
  https://dronebotworkshop.com
*/
 
// Include required libraries
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
 
void setup() {
    // Start Serial Port
    Serial.begin(115200);
    
    // Start microSD Card
    if(!SD.begin())
    {
      Serial.println("Error accessing microSD card!");
      while(true); 
    }
    
    // Setup I2S 
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    
    // Set Volume
    audio.setVolume(2);
    
    // Open music file
    audio.connecttoFS(SD,"/audio/alarm.wav");
    
}
 
void loop()
{
    audio.loop();
    if (!audio.isRunning()) {
      audio.connecttoFS(SD,"/audio/alarm.mp3");
      Serial.println("Starting audio playback again");
    }
}
