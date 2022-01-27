#include "combattant.h"

// instance heals "i" hit points
void Combattant::Heal(int i) {
  _hitPoints += i;
  if (_hitPoints > _maxHitPoints) { _hitPoints = _maxHitPoints;}
};

// instance takes "i" points of damage
void Combattant::TakeDamage(int i) {
  _hitPoints -= i;
  if (_hitPoints <= 0) { alive = false; }
};

// initialization method - could be replaced by a non-default constructor
void Combattant::InitStats(int maxHP, int AT, int DE, int AG, int XP, Faction faction, std::string name) {
    _attack_base = AT;
    _defense_base = DE;
    _agility = AG;
    _hitPoints = maxHP;
    _maxHitPoints = maxHP;
    _faction = faction;
    _name = name;
    _XPvalue = XP;
};

// check if combattant is allowed to move
bool Combattant::isMyTurnToMove() 
{
  ++_turnCounterMove;
  ++_turnCounterAttack;        
  if (_turnCounterMove > moveBaseSpeed_- _agility) {    
    _turnCounterMove = 0;
    return true;
  }
  return false;
}

// check if combattant is allowed to attack
bool Combattant::isMyTurnToAttack() 
{
  if (_turnCounterAttack > (moveBaseSpeed_ - _agility) * moveStepsPerCombatRound_) {
    _turnCounterAttack = 0;
    return true;
  }
  return false;
}
