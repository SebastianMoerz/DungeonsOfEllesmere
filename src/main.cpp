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
  constexpr std::size_t kGridMargin{1}; 
  constexpr std::size_t kNumberOfOpponents{6}; 
  //constexpr std::size_t kInitialOpponentSpeed{15};        // number of loops per step, i.e. the lower, the faster
  //constexpr std::size_t kInitialOpponentAttackSpeed{4};   // number of move steps per attack step, i.e. the lower, the faster
  //constexpr std::size_t kPlayerSpeed{10};                 // min number of loops between player steps, i.e. the lower, the faster
  constexpr std::size_t kMinStartDistance{10};            // minimum allowed distance between player and opponent at the start of the game

  Renderer renderer(kScreenWidth, kScreenHeight, kGridWidth, kGridHeight);
  Controller controller;
  Game game(kGridWidth, kGridHeight, kGridMargin, kNumberOfOpponents, kMinStartDistance);
  game.Run(controller, renderer, kMsPerFrame);
  std::cout << "Game has terminated successfully!\n";
  return 0;
}