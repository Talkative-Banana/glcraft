#pragma once
#include <SFML/Audio.hpp>
#include <SFML/Config.hpp>
#include <iostream>
#include <string.h>
#include "Constants.hpp"

class GameSound {
public:
  explicit GameSound(const std::string &path) {
    auto res = sound_buffer.loadFromFile(path);
    if (!res) {
      std::cerr << "Audio file at " << path << " not found\n";
    }
    sound = sf::Sound(sound_buffer);
    sound.setPosition({0.0, 0.0, 0.0});
    sound.setPitch(1.0f);
    sound.setVolume(100.0f); // original volume
    sound.setMinDistance(50.0f);
    sound.setAttenuation(0.5f);
#if SFML_VERSION_MAJOR >= 3
    sound.setLooping(true);
#else
    sound.setLoop(true);
#endif
    sound.setRelativeToListener(true);
    sound.play();
  }

  // Use it to get and set property of sound
  sf::Sound &get_sound() { return sound; }

  // Helper function to get status of sound across versions
  SOUNDSTATUS getSoundStatus(){
#if SFML_VERSION_MAJOR >= 3
      switch (sound.getStatus()) {
        case sf::SoundSource::Status::Stopped: return SOUNDSTATUS::STOPPED;
        case sf::SoundSource::Status::Paused:  return SOUNDSTATUS::PAUSED;
        case sf::SoundSource::Status::Playing: return SOUNDSTATUS::PLAYING;
      }
#else
      switch (sound.getStatus()) {
        case sf::SoundSource::Stopped: return SOUNDSTATUS::STOPPED;
        case sf::SoundSource::Paused:  return SOUNDSTATUS::PAUSED;
        case sf::SoundSource::Playing: return SOUNDSTATUS::PLAYING;
      }
#endif
      return SOUNDSTATUS::UNKNOWN;
  }

private:
  sf::SoundBuffer sound_buffer;
  sf::Sound sound{sound_buffer};
};
