#include "Game.hpp"

int main(int argc, char **argv) {
  // Create game object
  Game game("savePath");
  // holy loop
  game.run();
  // delete game
  game.shutdown();
  return 0;
}
