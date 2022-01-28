#include "player.h"
#include <iostream>
#include <algorithm>

// calculate new player position based on user input
SDL_Point Player::tryMove() 
{ 
  // temporary position
  int x = this->GetPosition().x;
  int y = this->GetPosition().y;
  // calculate requested move
  switch (direction) {
    case Direction::kNone:
      break;

    case Direction::kUp:
      y -= 1;
      break;

    case Direction::kDown:
      y += 1;
      break;void DisplayInventory();

    case Direction::kLeft:
      x -= 1;
      break;

    case Direction::kRight:
      x += 1;
      break;
  }
  // reset direction after moving
  direction = Direction::kNone;

  SDL_Point requestedPosition{x,y};
  return requestedPosition;
}

// add item to inventory without text suppression
void Player::receiveItem(std::unique_ptr<InventoryItem> item) {  
  receiveItem(std::move(item), false);
}

// select item number "i" from inventory without text suppression
void Player::SelectItem(int i) {  
  SelectItem(i, false);
 }

// add item to inventory
void Player::receiveItem(std::unique_ptr<InventoryItem> item, bool SuppressText) {
  
  // if the inventory already contains a similar item, increment the item count
  bool alreadyInInventory = false;
  for (auto& thing : _inventory) {
    if (thing->name == item->name) {
      alreadyInInventory = true;
      thing->number += item->number;
    }
  }

  // check and set game control flags
  if (item->isKey) { _hasKey = true; }
  if (item->isMcGuffin) { _hasMcGuffin = true; }
   
  auto plural = (item->number == 1) ? "" : "s";

  if (!SuppressText) { std::cout << std::to_string(item->number) << " " << item->name << plural << " added to inventory" << std::endl; }

  // if the item is not already in inventory, move it there
  if (!alreadyInInventory) {
    _inventory.emplace_back(std::move(item));
  }    
}


// Display the content of the player's inventory
void Player::DisplayInventory() {

  if (_inventory.empty()) {
    std::cout << "---------------" << std::endl;
    std::cout << "Your inventory is empty" << std::endl;
  }
  else {
    int i{0};
    std::cout << "---------------" << std::endl;
    std::cout << "Your inventory contains:" << std::endl;
    std::cout << "('*' indicates equipped items)" << std::endl;
    for (std::unique_ptr<InventoryItem> &item : _inventory) {
      i++;
      auto plural = (item->number == 1) ? "" : "s";          
      auto marker = ((item.get() == _equipped_weapon) || (item.get() == _equipped_armor)) ? " *" : "";
      std::cout << "#" << i << ": " << item->name << plural << " (" << item->number << ")" << marker << std::endl;
    }
    std::cout << "Press item number to equip or use" << std::endl;
  }  
}
  
// equip or use item number "i" from inventory
void Player::SelectItem(int i, bool suppressText) {
  
  if (_inventory.empty()) { return; }
  if ( i > _inventory.size()) { return; }

  InventoryItem* item = _inventory.at(i-1).get();
  if (item->isWeapon) { 
    _equipped_weapon = item; 
    if (!suppressText) { 
      std::cout << "---------------" << std::endl;
      std::cout << _equipped_weapon->name << " equipped as primary weapon" << std::endl; 
    }
    return; 
  }
  if (item->isArmor) { 
    _equipped_armor = item; 
    if (!suppressText) { 
      std::cout << "---------------" << std::endl;
      std::cout << _equipped_armor->name << " equipped as armor" << std::endl; 
    }
    return; 
  }
  if (item->healing > 0) { 
    Heal(item->healing);
    if (!suppressText) { 
      std::cout << "---------------" << std::endl;
      std::cout << "You healed " << item->healing << " hit points" << std::endl; 
    }
    if ( item->isSingleUseItem ) { DeleteFromInventory(item, 1); }      
    return; 
  }  

  if (item->name == "Torch") { 
    AddTimedEffect(0, "You light up a torch", 2);
    AddTimedEffect(10000, "Your torch burns low", 1);
    AddTimedEffect(12000, "With a last little flicker the torch light expires", 0);
    if ( item->isSingleUseItem ) { DeleteFromInventory(item, 1); } 
    return;
  }

  if (!suppressText) { 
    std::cout << "---------------" << std::endl;
    std::cout << "Item can not be equipped or used" << std::endl; 
  }
}

// remove item(s) from inventory
void Player::DeleteFromInventory(InventoryItem *toDelete, int number) {
  // check if enough instances of the item are available
  if (toDelete->number > number) {
    toDelete->number -= number;
    return;
  }
  // if not, purge from inventory
  for (std::unique_ptr<InventoryItem> & item : _inventory) { 
    auto it = std::find_if(_inventory.begin(), _inventory.end(), [toDelete](const std::unique_ptr<InventoryItem>& item) {return item->name == toDelete->name;});
    if (it != _inventory.end()) {
      _inventory.erase(it); 
      return;     
    }
  }
}

// display player status
void Player::DisplayStatus() {
  std::string status = (GetHP() < GetMaxHP()) ? "wounded" : "in perfect health";
  std::cout << "---------------" << std::endl;
  std::cout << "You are " << status << " (Hitpoints: " << GetHP() << "/" << GetMaxHP() << ")" << std::endl; 
}

// add xp to player's total experience
void Player::ReceiveXP (int xp) { 
  _XP += xp;
  std::cout << "You receive " << xp << " experience points (Total: " << _XP << ")" << std::endl;
}

// redefinition of virtual function of class combattant
int Player::GetAttackValue () {
  if (_equipped_weapon == nullptr) { return GetAttackBase(); }
  return GetAttackBase() + _equipped_weapon->attack_mod;
};

// redefinition of virtual function of class combattant
int Player::GetDefenseValue () { 
  if (_equipped_armor == nullptr) { return GetDefenseBase(); }
  return GetDefenseBase() + _equipped_armor->defense_mod;
};

// add new timed effect
void Player::AddTimedEffect(int timer, std::string msg, int mod) {
    _timedEffects.emplace_back(std::make_unique<TimedEffect>());
    _timedEffects.back()->timer= timer;
    _timedEffects.back()->mod = mod;
    _timedEffects.back()->msg = msg;
}


// update timed effects
void Player::UpdateEffects() {
    // check timers
    for (std::unique_ptr<TimedEffect> & effect : _timedEffects) {
        ++(effect->counter);
        if (effect->counter > effect->timer) {
            // apply effect
            _visionMod = effect->mod;
            std::cout << "---------------" << std::endl;
            std::cout << effect->msg << std::endl;
            // mark as expired
            effect->expired = true;
        }
    }    
    //remove expired effects    
    while (true) {
        auto it = std::find_if(_timedEffects.begin(), _timedEffects.end(), [](std::unique_ptr<TimedEffect> &effect){ return effect->expired;});
        if (it != _timedEffects.end()) { _timedEffects.erase(it); }
        else { break; }
    }    
    
  }

// return current player vision e.g. for rendering
Player::Vision Player::GetVision() { 
  
  if (_visionMod == 0) { return _vision; }
  
  if (_visionMod == 1) {
    switch (_vision) {
      case Player::Vision::kDaylight :
        return Player::Vision::kDaylight;
      case Player::Vision::kCavern :
        return Player::Vision::kDaylight;
      case Player::Vision::kDark1 :
        return Player::Vision::kCavern;
      case Player::Vision::kDark2 :
        return Player::Vision::kDark1;
      case Player::Vision::kDark3 :
        return Player::Vision::kDark2;
    }
  }

  if (_visionMod == 2) {
    switch (_vision) {
      case Player::Vision::kDaylight :
        return Player::Vision::kDaylight;
      case Player::Vision::kCavern :
        return Player::Vision::kDaylight;
      case Player::Vision::kDark1 :
        return Player::Vision::kDaylight;
      case Player::Vision::kDark2 :
        return Player::Vision::kCavern;
      case Player::Vision::kDark3 :
        return Player::Vision::kDark1;
    }
  }
  
  // else not implemented, return default
  return Player::Vision::kDaylight;
}
