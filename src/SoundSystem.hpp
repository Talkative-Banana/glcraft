#include "GameSound.hpp"
#include <unordered_map>

class SoundSystem {
public:
  SoundSystem() = default;

  void AddSound(std::string id, const GameSound &gamesound) {
    sounds.emplace(id, gamesound);
  }

  GameSound &GetSound(std::string id) { return sounds.at(id); }

private:
  std::unordered_map<std::string, GameSound> sounds;
};
