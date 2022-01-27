#ifndef COMBATTANT_H
#define COMBATTANT_H

#include <string>

// base class for objects that can engage in combat
class Combattant {
    public:
        enum class Faction { kNDEF, kFriendly, kNeutral, kHostile };

        // initialization method - could be replaced by a non-default constructor
        void InitStats(int maxHP, int AT, int DE, int AG, int XP, Faction faction, std::string name);
        
        bool alive{true};

        // main combat functions
        void Heal(int i);           // instance heals "i" hit points
        void TakeDamage(int i);     // instance takes "i" points of damage
        bool isMyTurnToMove();
        bool isMyTurnToAttack();        
    
        // setters & getters
        void SetFaction(Faction faction) { _faction=faction; }
        Faction GetFaction() { return _faction; }        
        std::string GetName () { return _name; }
        int GetXPValue () { return _XPvalue; }        
        int GetAttackBase() { return _attack_base; } 
        int GetDefenseBase() { return _defense_base;}
        int GetMaxHP() { return _maxHitPoints;}
        int GetHP() { return _hitPoints;}

        // virtual as long as NPCs don't have any items that alter their stats
        virtual int GetAttackValue () = 0;
        virtual int GetDefenseValue () = 0;

        // how many frames must pass before next turn?
        static int constexpr moveBaseSpeed_ = 20;
        static int constexpr moveStepsPerCombatRound_ = 4;

    private:
        std::string _name{""};
        
        // combat stats
        Faction _faction = Faction::kNDEF;
        int _maxHitPoints{1};
        int _hitPoints{1};
        int _attack_base{0}; 
        int _defense_base{0};        
        int _agility{0};
        int _XPvalue{0};        // how much XP will the player gain for defeating this opponent?      

        // counter
        int _turnCounterMove{0};
        int _turnCounterAttack{0};
};

#endif

