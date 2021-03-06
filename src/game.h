#ifndef GAME_H
#define GAME_H

#include <random>
#include <vector>
#include <memory>

#include "SDL.h"
#include "controller.h"
#include "renderer.h"
#include "player.h"
#include "opponent.h"
#include "entity.h"
#include "interactive_entity.h"
#include "combattant.h"
#include "door.h"
#include "event.h"
#include "tiletypes.h"


class Game {
 public:
  // constructor
  Game(std::size_t grid_width, std::size_t grid_height);

  // main method of this class
  void Run(Controller const &controller, Renderer &renderer, std::size_t target_frame_duration);

 private:
  // check if a position is on the game map
  bool IsOnMap(SDL_Point point);

  // overloaded function for collision detection. returns true if collision (or pointer to colliding object, depending on return type)
  // ROOM FOR IMPROVEMENT: #2 and #3 could be replaced by a template
  bool DetectCollision(SDL_Point point, Entity *entity);
  bool DetectCollision(SDL_Point point, std::vector<std::unique_ptr<Entity>> &entities);
  bool DetectCollision(SDL_Point point, std::vector<std::unique_ptr<InteractiveE>> &entities);
  Opponent* DetectCollision(SDL_Point point, std::vector<std::unique_ptr<Opponent>> &opponents);
  Door* DetectCollision(SDL_Point point, std::vector<std::unique_ptr<Door>> &doors);
    
  // same as above, but invokes Interact() method in colliding object
  bool DetectCollisionAndInteract(Player *player, SDL_Point point, std::vector<std::unique_ptr<InteractiveE>> &entities);
  void TriggerMapEvents(Player *player, std::vector<std::unique_ptr<MapEvent>> &events);
  
  // get a representation of the game map where each element is either "kNone" or "kObstacle"
  std::vector<std::vector<Entity::Type>> GetMapOfObstacles();

  // calculate damage dealt by attacker
  void HandleFight (Combattant* attacker, Combattant* defender);
  
  // game map data
  std::vector<std::vector<MapTiles::VicinityTileType>> _vicinitymap{};
  std::vector<std::vector<MapTiles::Type>> _rendermap{};  
  std::vector<std::vector<Entity::Type>> _obstaclemap{}; // not used yet
  std::size_t _grid_max_x;
  std::size_t _grid_max_y;

  // vectors for objects on the game map
  std::vector<std::unique_ptr<Entity>> _wall;
  std::vector<std::unique_ptr<Opponent>> _opponents;
  std::vector<std::unique_ptr<InteractiveE>> _npcs;
  std::vector<std::unique_ptr<InteractiveE>> _treasure;
  std::vector<std::unique_ptr<Door>> _doors;    
  std::vector<std::unique_ptr<MapEvent>> _events;

  
  // no pointer, since number of players is always one
  Player _player;    
    
  // random number engine
  std::random_device dev;
  std::mt19937 engine;
  std::uniform_int_distribution<int> random_w;
  std::uniform_int_distribution<int> random_h;

  // game & movement control
  bool _paused{false};
  bool _won{false};
  bool _pathBlocked{false};  
  void Update();    

  // setting up the game
  void SetUpPlayer(int x, int y);  
  void SetUpGameMap(std::string filepath);
  void PlaceOpponents(); 
  void PlaceTreasure();
  void PlaceNPCs();  
  void PlaceDoors();
  void PlaceEvents();
  void WelcomeMessage();
  
  // removing erased objects from data
  void CleanUpErasedEntities();

};

#endif