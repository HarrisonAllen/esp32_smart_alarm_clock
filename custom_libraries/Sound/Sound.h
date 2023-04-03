/*
    Sound.h - Library for controlling sound files via SD card
*/
#ifndef Sound_h
#define Sound_h

#include "Arduino.h"
#include "Audio.h"
#include "SD.h"

// I2S Connections
#define I2S_DOUT      27
#define I2S_BCLK      14
#define I2S_LRC       12

#define MAX_VOLUME    21
#define MIN_VOLUME    0

// TODO:
//  - Consider adding a lock, for example so that the alarm doesn't get overidden by another sound

class Sound {
    public:
        // Control Functions
        Sound(Audio * audio);
        void begin();
        void loop();
        void play();
        void stop();
        void playOnce(char * soundFile);
        // Setters
        int setVolume(int volume);
        int changeVolume(int change);
        int volumeUp(int change = 1);
        int volumeDown(int change = 1);
        void setSoundFile(char * soundFile);
        void setRepeating(bool shouldRepeat);
        // Getters
        int getVolume();
        bool isPlaying();
        bool isRepeating();
        Audio * getAudio();
    private:
        // Variables
        Audio *_audio;
        bool _repeat = false;
        bool _playing = false;
        char _filename[100] = "/audio/pop.mp3";
};


#endif