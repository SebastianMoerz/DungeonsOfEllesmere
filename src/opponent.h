#ifndef OPPONENT_H
#define OPPONENT_H

#include <vector>
#include <math.h>
#include "SDL.h"
#include "entity.h"
#include "combattant.h"

#include <random>
#include <string>
#include <memory>

// class definition of hostile NPCs
class Opponent : public Entity, public Combattant {
  public:
    
    // enum for the opponents state machine
    enum class State { kDead, kIdle, kSearching, kEngaging };

    // constructor
    Opponent(int x, int y, std::size_t mvSpd, std::size_t atSpd, Type type) : Entity(x, y, type), _movementSpeed(mvSpd), _attackSpeed(atSpd) { InitStats(8, 6, 6, 15, Faction::kHostile, "Orc"); }   

    // movement 
    bool isMyTurnToMove();
    SDL_Point BrownianMotion();
    SDL_Point tryMove(SDL_Point nextStepTowardPlayer, SDL_Point playerPosition); 
    void UpdateStateMachine(SDL_Point nextStepTowardPlayer, SDL_Point playerPosition);

    // combat - definition of virtual functions of class Combattant
    bool isMyTurnToAttack();   
    int GetAttackValue () {return GetAttackBase();};
    int GetDefenseValue () { return GetDefenseBase();};
    
    // drop a randomized item if opponent is defeated
    std::unique_ptr<InventoryItem> DropLoot(); 
    
   
  private:
    State _state{State::kIdle};   // NPC state machine
    int _turnCounter{0};          // game loops since last turn
    int _stepsSinceLastAttack{0};
    int _movementSpeed;           // game loops per movement
    int _attackSpeed;             // move steps per attack
    int _perception{10};          // detection threshold for distance to player

    // helper function to check if instance has detected the player      
    int CalculateDistance(SDL_Point start, SDL_Point target);  
};

#endif