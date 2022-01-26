#ifndef DOOR_H
#define DOOR_H

#include "entity.h"
#include "player.h"

class Door {
    public:
        enum class State { kLocked, kClosed, kOpen };    
        enum class DoorType { kRegular, kSecret, kDiscovered };

        Door(int x, int y, bool horizontal, bool secret, bool locked);
        void Interact(Player *player);        

        SDL_Point GetWingPosition() { return _wing.GetPosition(); }
        SDL_Point GetAnchorPosition() { return _anchor.GetPosition(); }
        DoorType GetDoorType() { return _type; }
  

    private:
        void OpenDoor();        
        Entity _anchor;
        Entity _wing;
        State _state{State::kClosed};
        DoorType _type{DoorType::kRegular};       
        bool _horizontal;
};

#endif