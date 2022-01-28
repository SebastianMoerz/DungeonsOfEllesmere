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

// ------------------------------------------------
// DEBUG RENDER : MAP WIREFRAME WITHOUT FOG OF WAR
// ------------------------------------------------
void Renderer::DebugRender(Player &player, std::vector<std::unique_ptr<InteractiveE>> &treasure, std::vector<std::unique_ptr<Entity>> &wall, std::vector<std::unique_ptr<Door>> &doors,
                      std::vector<std::unique_ptr<Opponent>> &opponents, std::vector<std::unique_ptr<InteractiveE>> &npcs, std::vector<std::unique_ptr<MapEvent>> &events, bool clearscreen) {
  SDL_Rect block;
  block.w = screen_width / grid_width;
  block.h = screen_height / grid_height;

  SDL_SetRenderDrawBlendMode(sdl_renderer, SDL_BLENDMODE_BLEND);

  // Clear screen
  if (clearscreen) {
    SDL_SetRenderDrawColor(sdl_renderer, 0x1E, 0x1E, 0x1E, 0xFF);
    SDL_RenderClear(sdl_renderer);
  }

  // Render treasure
  if (!treasure.empty()) {    
    for (std::unique_ptr<InteractiveE> &item : treasure) {
      if (item->GetType() == Entity::Type::kChest ) { SDL_SetRenderDrawColor(sdl_renderer, 0xAB, 0x60, 0x43, 0xFF); }
      else if (item->GetType() == Entity::Type::kLoot ) { SDL_SetRenderDrawColor(sdl_renderer, 0x00, 0xF0, 0xEE, 0xFF); }
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
      if (door->GetDoorType() == Door::DoorType::kRegular) {SDL_SetRenderDrawColor(sdl_renderer, 0x9B, 0x50, 0x23, 0xFF);}
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

  // Render events
  if (!events.empty()) {
    std::vector<SDL_Point> eventarea{};
    SDL_SetRenderDrawColor(sdl_renderer, 0xFF, 0xAE, 0xC9, 0x55);

    for (std::unique_ptr<MapEvent> &event : events) {
      eventarea = event->GetArea();
      for (SDL_Point point : eventarea) {        
        block.x = point.x * block.w;
        block.y = point.y * block.h;
        SDL_RenderFillRect(sdl_renderer, &block);        
      }
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



// --------------------------------------------------------------------------------
// RENDER COLORED GAME MAP WITH FOG OF WAR, APPLY ALPHA ACCORDING TO PLAYER VISION
// --------------------------------------------------------------------------------

void Renderer::Render(Player &player, std::vector<std::unique_ptr<InteractiveE>> &treasure, std::vector<std::unique_ptr<Entity>> &wall, std::vector<std::unique_ptr<Door>> &doors,
                      std::vector<std::unique_ptr<Opponent>> &opponents, std::vector<std::unique_ptr<InteractiveE>> &npcs, std::vector<std::vector<MapTiles::VicinityTileType>> &vicinitymap, std::vector<std::vector<MapTiles::Type>> &rendermap) {
  
  // define brush for painting squares
  SDL_Rect block;
  block.w = screen_width / grid_width;
  block.h = screen_height / grid_height;
  
  // activate alpha blending
  int _alpha = 0xFF;
  SDL_SetRenderDrawBlendMode(sdl_renderer, SDL_BLENDMODE_BLEND);
  
  // Clear screen
  SDL_SetRenderDrawColor(sdl_renderer, 0x1E, 0x1E, 0x1E, 0xFF);
  SDL_RenderClear(sdl_renderer);


  // render map terrain in player's vision range
  for (int x = 0; x < vicinitymap.size(); x++) {
    for (int y = 0; y < vicinitymap[0].size(); y++) {   
  
      // make sure tile is on screen and within players vision range
      if (isOnRenderMap({x,y}, player.GetPosition(), rendermap) && vicinitymap[x][y] != MapTiles::VicinityTileType::kOutside ) { 
        
        if (vicinitymap[x][y] == MapTiles::VicinityTileType::kInside) { _alpha = 0xFF; }
        if (vicinitymap[x][y] == MapTiles::VicinityTileType::kFringe) { _alpha = 0x55; }

        block.x = (x + player.GetPosition().x - 9) * block.w;
        block.y = (y + player.GetPosition().y - 9) * block.h;
        SDL_SetRenderDrawColor(sdl_renderer, 0xAB, 0x60, 0x43, 0xFF);
        
        if (rendermap[x + player.GetPosition().x - 9][y + player.GetPosition().y - 9] == MapTiles::Type::kFloor) { SDL_SetRenderDrawColor(sdl_renderer, 0x44, 0x22, 0x00, _alpha); }
        if (rendermap[x + player.GetPosition().x - 9][y + player.GetPosition().y - 9] == MapTiles::Type::kOuterWall) { SDL_SetRenderDrawColor(sdl_renderer, 0x99, 0x99, 0x99, _alpha); }
        if (rendermap[x + player.GetPosition().x - 9][y + player.GetPosition().y - 9] == MapTiles::Type::kInnerWall) { SDL_SetRenderDrawColor(sdl_renderer, 0x55, 0x55, 0x55, _alpha); }
        if (rendermap[x + player.GetPosition().x - 9][y + player.GetPosition().y - 9] == MapTiles::Type::kBedrock) { SDL_SetRenderDrawColor(sdl_renderer, 0x22, 0x22, 0x22, _alpha); }
        if (rendermap[x + player.GetPosition().x - 9][y + player.GetPosition().y - 9] == MapTiles::Type::kGras) { SDL_SetRenderDrawColor(sdl_renderer, 0x00, 0x7F, 0x00, _alpha); }
        SDL_RenderFillRect(sdl_renderer, &block);              
      }      
    }
  } 

  // Render treasure
  if (!treasure.empty()) {    
    for (std::unique_ptr<InteractiveE> &item : treasure) {
      
      SDL_Point vector = GetVector(item->GetPosition(), player.GetPosition());
      int x = vector.x + 9; // add player position offset
      int y = vector.y + 9; // add player position offset

      if (isOnVicinityMap(vector, vicinitymap)) {

        if (vicinitymap[x][y] == MapTiles::VicinityTileType::kOutside) { break; }
        if (vicinitymap[x][y] == MapTiles::VicinityTileType::kInside) { _alpha = 0xFF; }
        if (vicinitymap[x][y] == MapTiles::VicinityTileType::kFringe) { _alpha = 0x55; }
        
        // set color
        if (item->GetType() == Entity::Type::kChest ) { SDL_SetRenderDrawColor(sdl_renderer, 0xAB, 0x60, 0x43, _alpha); }
        else if (item->GetType() == Entity::Type::kLoot ) { SDL_SetRenderDrawColor(sdl_renderer, 0x00, 0xF0, 0xEE, _alpha); }
        else {SDL_SetRenderDrawColor(sdl_renderer, 0xFF, 0xCC, 0x00, _alpha);}

        block.x = item->GetPosition().x * block.w;
        block.y = item->GetPosition().y * block.h;
        SDL_RenderFillRect(sdl_renderer, &block);        
      }      
    }
  }

  // Render NPCs
  if (!npcs.empty()) {    
    for (std::unique_ptr<InteractiveE> &item : npcs) {
      
      SDL_Point vector = GetVector(item->GetPosition(), player.GetPosition());
      int x = vector.x + 9; // add player position offset
      int y = vector.y + 9; // add player position offset

      if (isOnVicinityMap(vector, vicinitymap)) {

        if (vicinitymap[x][y] == MapTiles::VicinityTileType::kOutside) { break; }
        if (vicinitymap[x][y] == MapTiles::VicinityTileType::kInside) { _alpha = 0xFF; }
        if (vicinitymap[x][y] == MapTiles::VicinityTileType::kFringe) { _alpha = 0x55; }
        
        SDL_SetRenderDrawColor(sdl_renderer, 0x00, 0xF0, 0xEE, _alpha);
        block.x = item->GetPosition().x * block.w;
        block.y = item->GetPosition().y * block.h;
        SDL_RenderFillRect(sdl_renderer, &block);        
      }      
    }
  }

  // Render doors
   if (!doors.empty()) { 
    SDL_Point vector;
    int x;
    int y;

    for (std::unique_ptr<Door> &door : doors) {

      // render anchor
      vector = GetVector(door->GetAnchorPosition(), player.GetPosition());
      x = vector.x + 9; // add player position offset
      y = vector.y + 9; // add player position offset

      if (isOnVicinityMap(vector, vicinitymap)) {

        if (vicinitymap[x][y] == MapTiles::VicinityTileType::kOutside) { break; }
        if (vicinitymap[x][y] == MapTiles::VicinityTileType::kInside) { _alpha = 0xFF; }
        if (vicinitymap[x][y] == MapTiles::VicinityTileType::kFringe) { _alpha = 0x55; }
        
        if (door->GetDoorType() == Door::DoorType::kRegular) {SDL_SetRenderDrawColor(sdl_renderer, 0xAB, 0x60, 0x43, _alpha);}
        if (door->GetDoorType() == Door::DoorType::kDiscovered) {SDL_SetRenderDrawColor(sdl_renderer, 0x77, 0x77, 0x77, _alpha);}
        if (door->GetDoorType() == Door::DoorType::kSecret) {SDL_SetRenderDrawColor(sdl_renderer, 0x99, 0x99, 0x99, _alpha);}
        
        block.x = door->GetAnchorPosition().x * block.w;
        block.y = door->GetAnchorPosition().y * block.h;
        SDL_RenderFillRect(sdl_renderer, &block);        
      }     

      // render wing
      vector = GetVector(door->GetWingPosition(), player.GetPosition());
      x = vector.x + 9; // add player position offset
      y = vector.y + 9; // add player position offset

      if (isOnVicinityMap(vector, vicinitymap)) {

        if (vicinitymap[x][y] == MapTiles::VicinityTileType::kOutside) { break; }
        if (vicinitymap[x][y] == MapTiles::VicinityTileType::kInside) { _alpha = 0xFF; }
        if (vicinitymap[x][y] == MapTiles::VicinityTileType::kFringe) { _alpha = 0x55; }
        
        if (door->GetDoorType() == Door::DoorType::kRegular) {SDL_SetRenderDrawColor(sdl_renderer, 0xAB, 0x60, 0x43, _alpha);}
        if (door->GetDoorType() == Door::DoorType::kDiscovered) {SDL_SetRenderDrawColor(sdl_renderer, 0x77, 0x77, 0x77, _alpha);}
        if (door->GetDoorType() == Door::DoorType::kSecret) {SDL_SetRenderDrawColor(sdl_renderer, 0x99, 0x99, 0x99, _alpha);}
        
        block.x = door->GetWingPosition().x * block.w;
        block.y = door->GetWingPosition().y * block.h;
        SDL_RenderFillRect(sdl_renderer, &block);        
      }  
    }
  }
  

   if (!opponents.empty()) {    
    for (std::unique_ptr<Opponent> &item : opponents) {
      
      SDL_Point vector = GetVector(item->GetPosition(), player.GetPosition());
      int x = vector.x + 9; // add player position offset
      int y = vector.y + 9; // add player position offset

      if (isOnVicinityMap(vector, vicinitymap)) {

        if (vicinitymap[x][y] == MapTiles::VicinityTileType::kOutside) { break; }
        if (vicinitymap[x][y] == MapTiles::VicinityTileType::kInside) { _alpha = 0xFF; }
        if (vicinitymap[x][y] == MapTiles::VicinityTileType::kFringe) { _alpha = 0x55; }
        
        SDL_SetRenderDrawColor(sdl_renderer, 0xFF, 0x00, 0x00, _alpha);
        block.x = item->GetPosition().x * block.w;
        block.y = item->GetPosition().y * block.h;
        SDL_RenderFillRect(sdl_renderer, &block);        
      }      
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


  // apply player vision brightness to full screen
  block.w = screen_width;
  block.h = screen_height;
  block.x = 0;
  block.y = 0;

  switch (player.GetVision())
  {
  case Player::Vision::kDaylight :    
    _alpha = 0x00;
    break;
  case Player::Vision::kCavern :
    _alpha = 0x55;
    break;
  case Player::Vision::kDark1 :
    _alpha = 0xAA;
    break;
  case Player::Vision::kDark2 :
    _alpha = 0xDD;
    break;
  case Player::Vision::kDark3 :
    _alpha = 0xEE;
    break;
  
  default:
    break;
  }
  SDL_SetRenderDrawColor(sdl_renderer, 0x00, 0x00, 0x00, _alpha);
  SDL_RenderFillRect(sdl_renderer, &block);
  
  // Update Screen
  SDL_RenderPresent(sdl_renderer);
}




// -----------------
// HELPER FUNCTIONS
// -----------------


bool Renderer::isOnVicinityMap(SDL_Point vectorToPlayer, std::vector<std::vector<MapTiles::VicinityTileType>> &map) {
  return (vectorToPlayer.x + 9 >=0 && vectorToPlayer.x + 9 < map.size() && vectorToPlayer.y + 9 >=0 && vectorToPlayer.y + 9 < map.size());
}

bool Renderer::isOnRenderMap(SDL_Point vector, SDL_Point playerPos, std::vector<std::vector<MapTiles::Type>> &map) {
  return (vector.x + playerPos.x - 9 >=0 && vector.x + playerPos.x - 9 < map.size() && vector.y + playerPos.y - 9 >=0 && vector.y + playerPos.y - 9 < map[0].size());
}
