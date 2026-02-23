#pragma once
#include <SFML/Audio.hpp>
#include <SFML/Config.hpp>
#include <iostream>
#include <string.h>

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

private:
  sf::SoundBuffer sound_buffer;
  sf::Sound sound{sound_buffer};
};
