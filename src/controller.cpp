#include <iostream>
#include <string>

#include "controller.h"
#include "SDL.h"
#include "player.h"

void Controller::HandleInput(bool &running, bool &paused,  Player &player) const {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT) {
      running = false;
    } else if (e.type == SDL_KEYDOWN) {
      // detect key presses
      switch (e.key.keysym.sym) {
        // game control
        case SDLK_p: { 
          std::string s = paused == false ? "Pause" : "Resume";
          std::cout << "---------------" << std::endl; 
          std::cout << s << " game" << std::endl;
          paused = paused == false ? true : false;
          }
          break;
        
        // movement
        case SDLK_UP:
          if (!paused) { player.direction = Player::Direction::kUp; };
          break;

        case SDLK_DOWN:
          if (!paused) { player.direction = Player::Direction::kDown; };
          break;

        case SDLK_LEFT:
          if (!paused) { player.direction = Player::Direction::kLeft; };
          break;

        case SDLK_RIGHT:
         if (!paused) { player.direction = Player::Direction::kRight; };
          break;

        // misc controls
        // display player status
        case SDLK_c : {
          player.DisplayStatus();
          break;
        }
        // display inventory
        case SDLK_i : {
          player.DisplayInventory();
          break;
        }
        // equip or use inventory item
        case SDLK_1 : {
          player.SelectItem(1);
          break;
        }
        case SDLK_2 : {
          player.SelectItem(2);
          break;
        }
        case SDLK_3 : {
          player.SelectItem(3);
          break;
        }
        case SDLK_4 : {
          player.SelectItem(4);
          break;
        }
        case SDLK_5 : {
          player.SelectItem(5);
          break;
        }
        case SDLK_6 : {
          player.SelectItem(6);
          break;
        }
        case SDLK_7 : {
          player.SelectItem(7);
          break;
        }
        case SDLK_8 : {
          player.SelectItem(8);
          break;
        }
        case SDLK_9 : {
          player.SelectItem(9);
          break;
        }
      }
    }
  }
}