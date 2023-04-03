/*
    Sound.cpp - Library for controlling sound files via SD card
*/

#include "Sound.h"

Sound::Sound(Audio * audio) {
    _audio = audio;
}

void Sound::begin() {
    _audio->setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    _audio->setVolume(5);
}

void Sound::loop() {
    _audio->loop();
    if (_playing && !_audio->isRunning()) {
        if (_repeat) {
            play();
        } else {
            _playing = false;
        }
    }
}

void Sound::play() {
    _playing = true;
    _audio->connecttoFS(SD, _filename);
}

void Sound::stop() {
    _playing = false;
    _audio->stopSong();
}

void Sound::setVolume(int volume) {
    _audio->setVolume(constrain(volume, 0, _audio->maxVolume()));
}

void Sound::changeVolume(int change) {
    setVolume(_audio->getVolume() + change);
}

void Sound::volumeUp(int change) {
    changeVolume(change);
}

void Sound::volumeDown(int change) {
    changeVolume(-change);
}

void Sound::setSoundFile(char * soundFile) {
    strcpy(_filename, soundFile);
}

void Sound::setRepeating(bool shouldRepeat) {
    _repeat = shouldRepeat;
}

int Sound::getVolume() {
    return _audio->getVolume();
}

bool Sound::isPlaying() {
    return _playing;
}

bool Sound::isRepeating() {
    return _repeat;
}

Audio * Sound::getAudio() {
    return _audio;
}
