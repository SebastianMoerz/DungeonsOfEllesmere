#include "renderer.h"
#include <iostream>
#include <string>


Renderer::Renderer(const std::size_t screen_width,
                   const std::size_t screen_height,
                   const std::size_t grid_width, const std::size_t grid_height)
    : screen_width(screen_width),
      screen_height(screen_height),
      grid_width(grid_width),
      grid_height(grid_height) {
  // Initialize SDL  
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "SDL could not initialize.\n";
    std::cerr << "SDL_Error: " << SDL_GetError() << "\n";
  }

  // Create Window
  sdl_window = SDL_CreateWindow("DARWIN - Eat or be eaten   ||    Score: 0", SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED, screen_width,
                                screen_height, SDL_WINDOW_SHOWN);

  if (nullptr == sdl_window) {
    std::cerr << "Window could not be created.\n";
    std::cerr << " SDL_Error: " << SDL_GetError() << "\n";
  }

  // Create renderer
  sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED);
  if (nullptr == sdl_renderer) {
    std::cerr << "Renderer could not be created.\n";
    std::cerr << "SDL_Error: " << SDL_GetError() << "\n";
  }
}

Renderer::~Renderer() {
  SDL_DestroyWindow(sdl_window);
  SDL_Quit();
}

// without fog of war
void Renderer::DebugRender(Player &player, std::vector<std::unique_ptr<InteractiveE>> &treasure, std::vector<std::unique_ptr<Entity>> &wall, std::vector<std::unique_ptr<Door>> &doors,
                      std::vector<std::unique_ptr<Opponent>> &opponents, std::vector<std::unique_ptr<InteractiveE>> &npcs) {
  SDL_Rect block;
  block.w = screen_width / grid_width;
  block.h = screen_height / grid_height;

  // Clear screen
  SDL_SetRenderDrawColor(sdl_renderer, 0x1E, 0x1E, 0x1E, 0xFF);
  SDL_RenderClear(sdl_renderer);

  // Render treasure
  if (!treasure.empty()) {    
    for (std::unique_ptr<InteractiveE> &item : treasure) {
      if (item->GetType() == Entity::Type::kChest ) { SDL_SetRenderDrawColor(sdl_renderer, 0xAB, 0x60, 0x43, 0xFF); }
      else {SDL_SetRenderDrawColor(sdl_renderer, 0xFF, 0xCC, 0x00, 0xFF);}
      block.x = item->GetPosition().x * block.w;
      block.y = item->GetPosition().y * block.h;
      SDL_RenderFillRect(sdl_renderer, &block);
    }
  }

  // Render NPCs
  if (!npcs.empty()) {
    SDL_SetRenderDrawColor(sdl_renderer, 0x00, 0xF0, 0xEE, 0xFF);
    for (std::unique_ptr<InteractiveE> &item : npcs) {
      block.x = item->GetPosition().x * block.w;
      block.y = item->GetPosition().y * block.h;
      SDL_RenderFillRect(sdl_renderer, &block);
    }
  }

  // Render wall
  if (!wall.empty()) {
    SDL_SetRenderDrawColor(sdl_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    for (std::unique_ptr<Entity> &brick : wall) {      
      block.x = brick->GetPosition().x * block.w;
      block.y = brick->GetPosition().y * block.h;
      SDL_RenderFillRect(sdl_renderer, &block);
    }
  }

  // Render doors
  if (!doors.empty()) {        
    for (std::unique_ptr<Door> &door : doors) {      
      if (door->GetDoorType() == Door::DoorType::kRegular) {SDL_SetRenderDrawColor(sdl_renderer, 0x7B, 0x30, 0x13, 0xFF);}
      if (door->GetDoorType() == Door::DoorType::kDiscovered) {SDL_SetRenderDrawColor(sdl_renderer, 0x99, 0x99, 0x99, 0xFF);}
      if (door->GetDoorType() == Door::DoorType::kSecret) {SDL_SetRenderDrawColor(sdl_renderer, 0xFF, 0xFF, 0xFF, 0xFF);}
      block.x = door->GetAnchorPosition().x * block.w;
      block.y = door->GetAnchorPosition().y  * block.h;
      SDL_RenderFillRect(sdl_renderer, &block);
      block.x = door->GetWingPosition().x * block.w;
      block.y = door->GetWingPosition().y  * block.h;
      SDL_RenderFillRect(sdl_renderer, &block);
    }
  }

  // Render opponents
  if (!opponents.empty()) {
    SDL_SetRenderDrawColor(sdl_renderer, 0xFF, 0x00, 0x00, 0xFF);
    for (std::unique_ptr<Opponent> &opponent : opponents) {
      block.x = opponent->GetPosition().x * block.w;
      block.y = opponent->GetPosition().y * block.h;
      SDL_RenderFillRect(sdl_renderer, &block);
    }
  }

  // Render player
  block.x = player.GetPosition().x * block.w;
  block.y = player.GetPosition().y * block.h;
  if (player.alive) {
    SDL_SetRenderDrawColor(sdl_renderer, 0x00, 0x7A, 0xCC, 0xFF);
  } else {
    SDL_SetRenderDrawColor(sdl_renderer, 0xFF, 0x00, 0x00, 0xFF);
  }
  SDL_RenderFillRect(sdl_renderer, &block);

  // Update Screen
  SDL_RenderPresent(sdl_renderer);
}

void Renderer::UpdateWindowTitle(int fps) {
  // use this line for debug output (fps)
  // std::string title{"Dungeons of Ellesmere - Quest for the Golden McGuffin || FPS: " + std::to_string(fps)}; 
  // use this line for output of player score only
  std::string title{"Dungeons of Ellesmere - Quest for the Golden McGuffin"};
  SDL_SetWindowTitle(sdl_window, title.c_str());
}





// with fog of war
void Renderer::Render(Player &player, std::vector<std::unique_ptr<InteractiveE>> &treasure, std::vector<std::unique_ptr<Entity>> &wall, std::vector<std::unique_ptr<Door>> &doors,
                      std::vector<std::unique_ptr<Opponent>> &opponents, std::vector<std::unique_ptr<InteractiveE>> &npcs, std::vector<std::vector<MapTiles::VicinityTileType>> &vicinitymap, std::vector<std::vector<MapTiles::Type>> &rendermap) {
  SDL_Rect block;
  block.w = screen_width / grid_width;
  block.h = screen_height / grid_height;

  SDL_SetRenderDrawBlendMode(sdl_renderer, SDL_BLENDMODE_BLEND);
  // Clear screen
  SDL_SetRenderDrawColor(sdl_renderer, 0x1E, 0x1E, 0x1E, 0xFF);
  SDL_RenderClear(sdl_renderer);

  //for each SDL_Point in playervicinity, get corresponding point from render map and render accordingly (if kIn - 100% alpha, if kFringe 50%, kOut: don't) 
  // PoC: refactor so that the map contains enities
  
  for (int x = 0; x < vicinitymap.size(); x++) {
    for (int y = 0; y < vicinitymap[0].size(); y++) {   
  
      block.x = (x + player.GetPosition().x - 9) * block.w;
      block.y = (y + player.GetPosition().y - 9) * block.h;
      SDL_SetRenderDrawColor(sdl_renderer, 0xAB, 0x60, 0x43, 0xFF);
      if (vicinitymap[x][y] == MapTiles::VicinityTileType::kInside) {
        // replace with function "isOnRenderMap"
        if (x + player.GetPosition().x - 9 >=0 && x + player.GetPosition().x - 9 < rendermap.size() && y + player.GetPosition().y - 9 >=0 && y + player.GetPosition().y - 9 < rendermap[0].size()) {
          if (rendermap[x + player.GetPosition().x - 9][y + player.GetPosition().y - 9] == MapTiles::Type::kFloor) { SDL_SetRenderDrawColor(sdl_renderer, 0x44, 0x22, 0x00, 0xFF); }
          if (rendermap[x + player.GetPosition().x - 9][y + player.GetPosition().y - 9] == MapTiles::Type::kOuterWall) { SDL_SetRenderDrawColor(sdl_renderer, 0x99, 0x99, 0x99, 0xFF); }
          if (rendermap[x + player.GetPosition().x - 9][y + player.GetPosition().y - 9] == MapTiles::Type::kInnerWall) { SDL_SetRenderDrawColor(sdl_renderer, 0x55, 0x55, 0x55, 0xFF); }
          if (rendermap[x + player.GetPosition().x - 9][y + player.GetPosition().y - 9] == MapTiles::Type::kBedrock) { SDL_SetRenderDrawColor(sdl_renderer, 0x22, 0x22, 0x22, 0xFF); }
          if (rendermap[x + player.GetPosition().x - 9][y + player.GetPosition().y - 9] == MapTiles::Type::kGras) { SDL_SetRenderDrawColor(sdl_renderer, 0x00, 0x7F, 0x00, 0xFF); }
          SDL_RenderFillRect(sdl_renderer, &block);
        }
      }
      if (vicinitymap[x][y] == MapTiles::VicinityTileType::kFringe) {
        // replace with function "isOnRenderMap"
        if (x + player.GetPosition().x - 9 >=0 && x + player.GetPosition().x - 9 < rendermap.size() && y + player.GetPosition().y - 9 >=0 && y + player.GetPosition().y - 9 < rendermap[0].size()) {
          if (rendermap[x + player.GetPosition().x - 9][y + player.GetPosition().y - 9] == MapTiles::Type::kFloor) { SDL_SetRenderDrawColor(sdl_renderer, 0x44, 0x22, 0x00, 0x55); }
          if (rendermap[x + player.GetPosition().x - 9][y + player.GetPosition().y - 9] == MapTiles::Type::kOuterWall) { SDL_SetRenderDrawColor(sdl_renderer, 0x99, 0x99, 0x99, 0x55); }
          if (rendermap[x + player.GetPosition().x - 9][y + player.GetPosition().y - 9] == MapTiles::Type::kInnerWall) { SDL_SetRenderDrawColor(sdl_renderer, 0x55, 0x55, 0x55, 0x55); }
          if (rendermap[x + player.GetPosition().x - 9][y + player.GetPosition().y - 9] == MapTiles::Type::kBedrock) { SDL_SetRenderDrawColor(sdl_renderer, 0x22, 0x22, 0x22, 0x55); }
          if (rendermap[x + player.GetPosition().x - 9][y + player.GetPosition().y - 9] == MapTiles::Type::kGras) { SDL_SetRenderDrawColor(sdl_renderer, 0x00, 0x7F, 0x00, 0x55); }
          SDL_RenderFillRect(sdl_renderer, &block);
        }
      }
    }
  } 

  // Render treasure
  if (!treasure.empty()) {    
    for (std::unique_ptr<InteractiveE> &item : treasure) {
      if (item->GetType() == Entity::Type::kChest ) { SDL_SetRenderDrawColor(sdl_renderer, 0xAB, 0x60, 0x43, 0xFF); }
      else {SDL_SetRenderDrawColor(sdl_renderer, 0xFF, 0xCC, 0x00, 0xFF);}
      block.x = item->GetPosition().x * block.w;
      block.y = item->GetPosition().y * block.h;
      SDL_RenderFillRect(sdl_renderer, &block);
    }
  }

  // Render NPCs
  if (!npcs.empty()) {

    SDL_SetRenderDrawColor(sdl_renderer, 0x00, 0xF0, 0xEE, 0xFF);
    for (std::unique_ptr<InteractiveE> &item : npcs) {
      SDL_Point vector = GetVector(item->GetPosition(), player.GetPosition());
      // move this check to function "isOnVicinityMap"
      if (vector.x + 9 >=0 && vector.x + 9 < vicinitymap.size() && vector.y + 9 >=0 && vector.y + 9 < vicinitymap.size()) {
        if  (vicinitymap[vector.x + 9][vector.y + 9] == MapTiles::VicinityTileType::kInside) {
          block.x = item->GetPosition().x * block.w;
          block.y = item->GetPosition().y * block.h;
          SDL_RenderFillRect(sdl_renderer, &block);
        }
      }      
    }
  }

/*
  // Render wall
  if (!wall.empty()) {
    SDL_SetRenderDrawColor(sdl_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    for (std::unique_ptr<Entity> &brick : wall) {      
      block.x = brick->GetPosition().x * block.w;
      block.y = brick->GetPosition().y * block.h;
      SDL_RenderFillRect(sdl_renderer, &block);
    }
  }

*/

  // Render doors
  if (!doors.empty()) {        
    for (std::unique_ptr<Door> &door : doors) {      
      if (door->GetDoorType() == Door::DoorType::kRegular) {SDL_SetRenderDrawColor(sdl_renderer, 0x7B, 0x30, 0x13, 0xFF);}
      if (door->GetDoorType() == Door::DoorType::kDiscovered) {SDL_SetRenderDrawColor(sdl_renderer, 0x99, 0x99, 0x99, 0xFF);}
      if (door->GetDoorType() == Door::DoorType::kSecret) {SDL_SetRenderDrawColor(sdl_renderer, 0xFF, 0xFF, 0xFF, 0xFF);}
      block.x = door->GetAnchorPosition().x * block.w;
      block.y = door->GetAnchorPosition().y  * block.h;
      SDL_RenderFillRect(sdl_renderer, &block);
      block.x = door->GetWingPosition().x * block.w;
      block.y = door->GetWingPosition().y  * block.h;
      SDL_RenderFillRect(sdl_renderer, &block);
    }
  }

  // Render opponents
  if (!opponents.empty()) {
    SDL_SetRenderDrawColor(sdl_renderer, 0xFF, 0x00, 0x00, 0xFF);
    for (std::unique_ptr<Opponent> &opponent : opponents) {
      block.x = opponent->GetPosition().x * block.w;
      block.y = opponent->GetPosition().y * block.h;
      SDL_RenderFillRect(sdl_renderer, &block);
    }
  }

  // Render player
  block.x = player.GetPosition().x * block.w;
  block.y = player.GetPosition().y * block.h;
  if (player.alive) {
    SDL_SetRenderDrawColor(sdl_renderer, 0x00, 0x7A, 0xCC, 0xFF);
  } else {
    SDL_SetRenderDrawColor(sdl_renderer, 0xFF, 0x00, 0x00, 0xFF);
  }
  SDL_RenderFillRect(sdl_renderer, &block);

  // Update Screen
  SDL_RenderPresent(sdl_renderer);
}