#ifndef ENTITY_H
#define ENTITY_H

#include "SDL.h"
#include "tiletypes.h"
#include <string>

// inventory items can be used by instances of Entity's child classes
struct InventoryItem {
    std::string name{""};
    int number{1};
    int attack_mod{0};
    int defense_mod{0};
    int healing{0};    
    
    bool isSingleUseItem{false};
    bool isWeapon{false};
    bool isArmor{false};    
    bool isKey{false};
    bool isMcGuffin{false};     
};

// base class for objects on the game map (no separate .cpp file due to trivial member definition)
class Entity {
 public:
  // used e.g. for rendering and for defining behavioral detail in child classes (currently not all types are used)
  enum class Type { kNone, kEvent, kObstacle, kDoor, kTreasure, kLoot, kChest, kNPC, kPlayer }; 

  // WIP: for obstacle tiles
  // enum class ExplorationStatus { kExplored, kUnexplored };

  // for movement
  enum class Direction { kUp, kDown, kLeft, kRight, kNone };              
    
  // constructor
  Entity(){};
  Entity(int x, int y, Type type) : _position({x,y}), _type(type) { if (type == Type::kEvent) { _blocksPath = false;} }
  
  // getters and setters
  void SetType(Type const &type) {_type = type;}
  Type GetType() {return _type;}
  void SetPosition(SDL_Point const &position) {_position = position;}
  SDL_Point GetPosition() {return _position;}
  void MarkForErasure () { _eraseFlag = true; }
  bool isMarkedForErasure () { return _eraseFlag; }
  void SetBlocksPath(bool blocksPath) { _blocksPath = blocksPath; }
  bool GetBlocksPath() { return _blocksPath; } 
  //void SetExplored() { _status = ExplorationStatus::kExplored; } 
  //bool isExplored() {return _status == ExplorationStatus::kExplored; }

private:
  Type _type{Type::kNone};
  //ExplorationStatus _status{ ExplorationStatus::kUnexplored };
  SDL_Point _position{0,0}; 
  bool _eraseFlag{false};     // if true, instance will be deleted at end of game loop 
  bool _blocksPath{true};     // if true, moving objects can't move at instance's position
};

#endif