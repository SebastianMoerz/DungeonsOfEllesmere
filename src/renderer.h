#ifndef RENDERER_H
#define RENDERER_H

#include <vector>
#include "SDL.h"
#include "player.h"
#include "entity.h"
#include "interactive_entity.h"
#include "opponent.h"
#include "door.h"
#include <memory>

class Renderer {
 public:
  Renderer(const std::size_t screen_width, const std::size_t screen_height,
           const std::size_t grid_width, const std::size_t grid_height);
  ~Renderer();

  void Render(Player &player, std::vector<std::unique_ptr<InteractiveE>> &treasure, std::vector<std::unique_ptr<Entity>> &wall, std::vector<std::unique_ptr<Door>> &doors,
              std::vector<std::unique_ptr<Opponent>> &opponents, std::vector<std::unique_ptr<InteractiveE>> &npcs);
  void UpdateWindowTitle(int fps);

 private:
  SDL_Window *sdl_window;
  SDL_Renderer *sdl_renderer;

  const std::size_t screen_width;
  const std::size_t screen_height;
  const std::size_t grid_width;
  const std::size_t grid_height;
};

#endif