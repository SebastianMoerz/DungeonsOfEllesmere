#ifndef MAP_EVENT_H
#define MAP_EVENT_H

#include "entity.h"
#include "player.h"
#include <memory>
#include <vector>
#include <iostream>
#include <string>
#include <algorithm>

class MapEvent : public Entity {

    public:

        MapEvent(int x, int y, std::string msg) : Entity(x,y,Entity::Type::kEvent), _msg(msg) { AddToArea(x,y); }        
        MapEvent(int x, int y, std::string msg, int xp, int dmg) : Entity(x,y,Entity::Type::kEvent), _msg(msg), _xp(xp), _dmg(dmg) { AddToArea(x,y); }

        void AddToArea(int x, int y) {
            if (!isInArea(x,y)) { _area.emplace_back(std::make_unique<Entity>(x,y,Entity::Type::kEvent)); }
        }
        
        void RemoveFromArea(int x, int y) {
            while (true) {
                auto it = std::find_if(_area.begin(), _area.end(), [&](const std::unique_ptr<Entity>& item) { return (item->GetPosition().x == x && item->GetPosition().y == y);});
                if (it == _area.end()) { return; }
                else { _area.erase(it); }
            }
        }

        bool isInArea(int x, int y) {
            for (std::unique_ptr<Entity> &entity : _area) {
                if (entity->GetPosition().x == x && entity->GetPosition().y == y) { return true; }
            }
            return false;
        }

        void Interact(Player *player) {
            std::cout << "---------------" << std::endl;
            std::cout << _msg << std::endl;

            if (_xp != 0) { player->ReceiveXP(_xp); std::cout << "You receive " << _xp << " experience points." << std::endl; }
            if (_dmg < 0) { player->Heal(_dmg); ; std::cout << "You heal " << _dmg << " hit points." << std::endl; }
            if (_dmg > 0) { player->TakeDamage (_dmg); std::cout << "You take " << _dmg << " points of damage." << std::endl;}

            this->MarkForErasure();
        }       

    private:

        std::vector<std::unique_ptr<Entity>> _area;
        std::string _msg{""};
        int _xp{0};
        int _dmg{0};

};

#endif
