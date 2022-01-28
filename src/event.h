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
        enum class EventType { kSingle, kPersitent, kIllumination, kCollectionQuest };

        // constructors for one-time events (Type kSingle)
        MapEvent(int x, int y, std::string msg) : Entity(x,y,Entity::Type::kEvent), _msg(msg) { AddToArea(x,y); }                  
        MapEvent(int x, int y, std::string msg, int xp, int dmg) : Entity(x,y,Entity::Type::kEvent), _msg(msg), _xp(xp), _dmg(dmg) { AddToArea(x,y); }

        // constructors for persistent events and side quests
        MapEvent(int x, int y, EventType type, std::string msg) : Entity(x,y,Entity::Type::kEvent), _msg(msg), _type(type) { AddToArea(x,y); }
        MapEvent(int x, int y, EventType type, std::string msg, int xp, int dmg) : Entity(x,y,Entity::Type::kEvent), _msg(msg), _xp(xp), _dmg(dmg), _type(type) { AddToArea(x,y); }

        // constructor for events that change map illumination (illumination events are persistent)
        MapEvent(int x, int y, Player::Vision vision, std::string msg) : Entity(x,y,Entity::Type::kEvent), _msg(msg), _type(EventType::kIllumination), _illumination(vision) { AddToArea(x,y); }       
        

        void AddToArea(int x, int y);
        
        void RemoveFromArea(int x, int y);

        bool isInArea(int x, int y);

        std::vector<SDL_Point> GetArea();

        void Interact(Player *player);    

    private:
        EventType _type{EventType::kSingle};
        std::vector<std::unique_ptr<Entity>> _area;
        std::string _msg{""};
        int _xp{0};
        int _dmg{0};        
        Player::Vision _illumination{Player::Vision::kDaylight};

};

#endif
