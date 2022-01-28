#include <iostream>
#include "controller.h"
#include "game.h"
#include "renderer.h"

int main() {
  constexpr std::size_t kFramesPerSecond{60};
  constexpr std::size_t kMsPerFrame{1000 / kFramesPerSecond};
  constexpr std::size_t kScreenWidth{1020};
  constexpr std::size_t kScreenHeight{780};
  constexpr std::size_t kGridWidth{51};
  constexpr std::size_t kGridHeight{39};  

  Renderer renderer(kScreenWidth, kScreenHeight, kGridWidth, kGridHeight);
  Controller controller;
  Game game(kGridWidth, kGridHeight);
  game.Run(controller, renderer, kMsPerFrame);
  std::cout << "Game has terminated successfully!\n";
  return 0;
}