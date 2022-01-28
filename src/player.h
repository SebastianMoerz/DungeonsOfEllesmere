#ifndef PLAYER_H
#define PLAYER_H

#include <memory>
#include <vector>
#include "SDL.h"
#include "entity.h"
#include "combattant.h"

// effects that can buff or nerf the player
struct TimedEffect {
    // use enum like this to enable other effect types
    // enum class EffectType { kIllumination, kDamage, kAttackMod, kDefenseMod, kSpeedMod }
    int timer;
    int counter{0};
    int mod;                // currently only illumination
    std::string msg;    
    bool expired{false};
};


// class definition of the player's avatar
class Player : public Entity, public Combattant {
 public:

  enum class Vision { kDaylight, kCavern, kDark1, kDark2, kDark3 };
  // constructors and assignment operators   
  Player() : Entity(_startX, _startY, Type::kPlayer) { InitStats(10, 6, 6, 8, 0, Faction::kNDEF, "Player"); }

  Player(const Player & source) = delete;           // delete copy constructor (unique pointers in inventory can't be copied)
  Player &operator=(const Player &source) = delete; // delete copy assignment operator (unique pointers in inventory can't be copied)

  // movement
  SDL_Point tryMove();                              // try to move the player into the direction indicated by "direction"
  Direction direction = Direction::kNone;  
  
  // game control methods
  bool hasKey() {return _hasKey; };                       // has the player found the key to the locked door?
  bool hasMcGuffin() { return _hasMcGuffin;};             // has the player found the main quest object?
  void SetQuestComplete() { _hasCompletedQuest = true; }
  bool GetQuestComplete() { return _hasCompletedQuest; }

  // map vision
  void SetVision(Vision vision) { _vision = vision; }
  Vision GetVision();
 
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

  // active effects
  void AddTimedEffect(int timer, std::string msg, int mod);
  void UpdateEffects();

  // misc
  void ReceiveXP (int xp);          // add xp to player's total XP
  int GetTotalXP () { return _XP; } 

 private: 
  // inventory
  std::vector<std::unique_ptr<InventoryItem>> _inventory;   // inventory items are stored here
  InventoryItem* _equipped_weapon{nullptr};                 // the weapon the player uses
  InventoryItem* _equipped_armor{nullptr};                  // the armor the player wears

  // active effects
  std::vector<std::unique_ptr<TimedEffect>> _timedEffects;
  
  // game control
  bool _hasKey{false};
  bool _hasMcGuffin{false};
  bool _hasCompletedQuest{false};

  // rendering
  Vision _vision{Vision::kDaylight};
  int _visionMod{0};

  // total player experience points
  int _XP{0}; 

  // default start position  
  int _startX{0};
  int _startY{0};
};

#endif




