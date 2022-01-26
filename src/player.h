#ifndef PLAYER_H
#define PLAYER_H

#include <memory>
#include <vector>
#include "SDL.h"
#include "entity.h"
#include "combattant.h"

// class definition of the player's avatar
class Player : public Entity, public Combattant {
 public:
  // constructors and assignment operators 
  //Player(int grid_width, int grid_height) : Entity(static_cast<int>(grid_width/2), static_cast<int>(grid_height/2), Type::kPlayer) { InitStats(10, 6, 6, 0, Faction::kNDEF, "Player"); }
  Player() : Entity(_startX, _startY, Type::kPlayer) { InitStats(10, 6, 6, 0, Faction::kNDEF, "Player"); }

  Player(const Player & source) = delete;           // delete copy constructor (unique pointers in inventory can't be copied)
  Player &operator=(const Player &source) = delete; // delete copy assignment operator (unique pointers in inventory can't be copied)

  // movement
  SDL_Point tryMove();                              // try to move the player into the direction indicated by "direction"
  Direction direction = Direction::kNone;
  bool isMyTurn();  
  void SetMaxPlayerSpeed (int speed) { _maxSpeed = speed; }
  
  // game control methods
  bool hasKey() {return _hasKey; };                       // has the player found the key to the locked door?
  bool hasMcGuffin() { return _hasMcGuffin;};             // has the player found the main quest object?
  void SetQuestComplete() { _hasCompletedQuest = true; }
  bool GetQuestComplete() { return _hasCompletedQuest; }

  // inventory methods
  void DisplayInventory(); 
  void DisplayStatus();
  void receiveItem(std::unique_ptr<InventoryItem> item);                      // move item to inventory and print message to console
  void receiveItem(std::unique_ptr<InventoryItem> item, bool suppressText);   // move item to inventory and decide if message shall be printed to console
  void SelectItem(int i);                                                     // select item from inventory and print message to console
  void SelectItem(int i, bool suppressText);                                  // select item from inventory and decide if message shall be printed to console
  void DeleteFromInventory(InventoryItem *item, int number);                  // delete "number" objects "item" from inventory

  // combat - definition of virtual functions of class Combattant
  int GetAttackValue ();
  int GetDefenseValue ();

  // misc
  void ReceiveXP (int xp);          // add xp to player's total XP
  int GetTotalXP () { return _XP; } 

 private: 
  // movement
  int _maxSpeed;
  long _turnCounter{0};

  // inventory
  std::vector<std::unique_ptr<InventoryItem>> _inventory;   // inventory items are stored here
  InventoryItem* _equipped_weapon{nullptr};                 // the weapon the player uses
  InventoryItem* _equipped_armor{nullptr};                  // the armor the player wears

  // game control
  bool _hasKey{false};
  bool _hasMcGuffin{false};
  bool _hasCompletedQuest{false};

  // total player experience points
  int _XP{0}; 

  // default start position  
  int _startX{0};
  int _startY{0};
};

#endif




