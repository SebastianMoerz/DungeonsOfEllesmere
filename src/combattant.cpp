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
void Combattant::InitStats(int maxHP, int AT, int DE, int XP, Faction faction, std::string name) {
    _attack_base = AT;
    _defense_base = DE;
    _hitPoints = maxHP;
    _maxHitPoints = maxHP;
    _faction = faction;
    _name = name;
    _XPvalue = XP;
};