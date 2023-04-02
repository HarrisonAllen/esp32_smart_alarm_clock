#include <Sound.h>

Audio audio;
Sound sound(&audio);

long soundTimer;
bool alarm_playing = true;
 
void setup() {
  // Start Serial Port
  Serial.begin(115200);
  
  // Start microSD Card
  if(!SD.begin())
  {
    Serial.println("Error accessing microSD card!");
    while(true); 
  }

  // Start sound
  sound.begin();
  sound.setSoundFile("/audio/alarm.mp3");
  sound.setRepeating(true);
  sound.play();
  soundTimer = millis();
}
 
void loop()
{
  sound.loop();
  if (alarm_playing && millis() - soundTimer > 5000) {
    sound.stop();
    sound.setSoundFile("/audio/epic_music.mp3");
    sound.volumeUp(10);
    sound.setRepeating(false);
    sound.play();
    alarm_playing = false;
  } else if (!sound.isPlaying()) {
    sound.stop();
    sound.setSoundFile("/audio/alarm.mp3");
    sound.volumeDown(10);
    sound.setRepeating(true);
    sound.play();
    soundTimer = millis();
    alarm_playing = true;
  }
}