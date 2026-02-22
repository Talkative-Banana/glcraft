#pragma once
#include <SFML/Audio.hpp>
#include <iostream>
#include <string.h>

class GameSound {
public:
  explicit GameSound(const std::string &path) {
    auto res = sound_buffer.loadFromFile(path);
    if (!res) {
      std::cerr << "Audio file at " << path << " not found\n";
    }

    sound.setBuffer(sound_buffer);
    sound.setPosition({0.0, 0.0, 0.0});
    sound.setPitch(1.0f);
    sound.setVolume(100.0f); // original volume
    sound.setMinDistance(50.0f);
    sound.setAttenuation(0.5f);
    sound.setLoop(true);
    sound.setRelativeToListener(false);
    sound.play();
  }

  // Use it to get and set property of sound
  sf::Sound &get_sound() { return sound; }

private:
  sf::Sound sound;
  sf::SoundBuffer sound_buffer;
};
