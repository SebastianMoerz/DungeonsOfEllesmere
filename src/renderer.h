#ifndef RENDERER_H
#define RENDERER_H

#include <vector>
#include "SDL.h"
#include "player.h"
#include "entity.h"
#include "interactive_entity.h"
#include "opponent.h"
#include "door.h"
#include "tiletypes.h"
#include <memory>

class Renderer {
 public:
  Renderer(const std::size_t screen_width, const std::size_t screen_height,
           const std::size_t grid_width, const std::size_t grid_height);
  ~Renderer();

  // no fog of war
  void DebugRender(Player &player, std::vector<std::unique_ptr<InteractiveE>> &treasure, std::vector<std::unique_ptr<Entity>> &wall, std::vector<std::unique_ptr<Door>> &doors,
              std::vector<std::unique_ptr<Opponent>> &opponents, std::vector<std::unique_ptr<InteractiveE>> &npcs);
  
  // WIP: with fog of war
  void Render(Player &player, std::vector<std::unique_ptr<InteractiveE>> &treasure, std::vector<std::unique_ptr<Entity>> &wall, std::vector<std::unique_ptr<Door>> &doors,
              std::vector<std::unique_ptr<Opponent>> &opponents, std::vector<std::unique_ptr<InteractiveE>> &npcs, std::vector<std::vector<MapTiles::VicinityTileType>> &vicinitymap, std::vector<std::vector<MapTiles::Type>> &rendermap);

  

  void UpdateWindowTitle(int fps);

 private:
  // helper function: straightforward position-delta calculation, taken from game-utils - include leads to linker error: REFACTOR!!
  SDL_Point GetVector (SDL_Point from, SDL_Point to) { return {to.x - from.x, to.y - from.y}; }
  
  SDL_Window *sdl_window;
  SDL_Renderer *sdl_renderer;

  const std::size_t screen_width;
  const std::size_t screen_height;
  const std::size_t grid_width;
  const std::size_t grid_height;
};

#endif