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

        void AddToArea(int x, int y);
        
        void RemoveFromArea(int x, int y);

        bool isInArea(int x, int y);

        void Interact(Player *player);

    private:

        std::vector<std::unique_ptr<Entity>> _area;
        std::string _msg{""};
        int _xp{0};
        int _dmg{0};

};

#endif
