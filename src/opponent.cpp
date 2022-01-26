#include <random>
#include "SDL.h"
#include "opponent.h"

bool Opponent::isMyTurnToMove() 
{
  ++_turnCounter;        
  if (_turnCounter>_movementSpeed) {
    ++_stepsSinceLastAttack;        // increment movement steps
    _turnCounter = 0;
    return true;
  }
  return false;
}

bool Opponent::isMyTurnToAttack() 
{
  if (_stepsSinceLastAttack >= _attackSpeed) {
    _stepsSinceLastAttack = 0;
    return true;
  }
  return false;
}

// try to find the next movement step of this instance
SDL_Point Opponent::tryMove(SDL_Point nextStepTowardPlayer, SDL_Point playerPosition) 
{      
  UpdateStateMachine(nextStepTowardPlayer, playerPosition);

  switch (_state) {
    case State::kDead:
      return GetPosition();         // return current position, i.e. don't move
    case State::kIdle:
      return GetPosition();         // return current position, i.e. don't move
    case State::kSearching:
      return BrownianMotion();      // random movement
    case State::kEngaging:
      return nextStepTowardPlayer;  // confirm movement toward player
  }
}

// simple random movement
SDL_Point Opponent::BrownianMotion()
{ 
  int direction = rand() % 5;       // using std::rand() only as proof of concept. of course other engines might be better suited
  int x = this->GetPosition().x;
  int y = this->GetPosition().y;
  // calculate requested move
  switch (direction) {
    case 0:  //none
      break;
     case 1:  //up
      y -= 1;
      break;
     case 2:  //down
      y += 1;
      break;
     case 3:  //left
      x -= 1;
      break;
     case 4:  //right
      x += 1;
      break;
  }      

  SDL_Point requestedPosition{x,y};
  return requestedPosition;     
};

// helper function: euclidean distance
int Opponent::CalculateDistance(SDL_Point start, SDL_Point target) {
    int dx = start.x - target.x;
    int dy = start.y - target.y;
    return static_cast<int>(sqrt(pow(dx,2.0)+pow(dy,2.0)));
}

// state machine definition
void Opponent::UpdateStateMachine(SDL_Point nextStepTowardPlayer, SDL_Point playerPosition) {

  if (!alive) { 
    if (_state != State::kDead) { _state = State::kDead; }
    return;
  } 
  // first, calculate distance to player & make perception roll
  int distToPlayer = CalculateDistance(playerPosition, this->GetPosition());
  int perceptionRoll = rand() % _perception;

  // if step is equal to current position, no valid path to player exists. stop moving.
  if (nextStepTowardPlayer.x == this->GetPosition().x && nextStepTowardPlayer.y == this->GetPosition().y) { 
    _state = State::kIdle;
  }
  // if player was spotted, engage
  else if (perceptionRoll >= distToPlayer) {
    _state = State::kEngaging;
  } 
  // if player was lost (dist > max perception), start searching 
  else if (_state == State::kEngaging && distToPlayer > _perception) {
    _state = State::kSearching;
  }
  // if path exists and player is approaching (but not spotted yet), start searching
  else if (_state == State::kIdle && distToPlayer <= _perception) {
    _state = State::kSearching;
  }
  // if path exists, but player wasn't spotted yet & is outside hearing range, remain in kIdle (no code)
}

// place random item on the map if defeated
std::unique_ptr<InventoryItem> Opponent::DropLoot() 
{
  int lootRoll = rand() % 5;
      
  switch (lootRoll) {
    case 0 ... 1: {
      // no item
      return nullptr;
    }
    case 2: {
      std::unique_ptr<InventoryItem> item = std::make_unique<InventoryItem>();
      item->name = "Rusty Knife";
      item->number = 1;
      item->attack_mod = 1;
      item->isWeapon = true;
      return item;    
    }
    case 3: {
      std::unique_ptr<InventoryItem> item = std::make_unique<InventoryItem>();
      item->name = "Moldy Bread";
      item->number = 1;
      item->healing = 2;
      item->isSingleUseItem = true;    
      return item;
    }
    case 4: {
      std::unique_ptr<InventoryItem> item = std::make_unique<InventoryItem>();
      item->name = "Gold Coin";
      item->number = rand() % 5 + 1;
      return item;
    }
  }
}   