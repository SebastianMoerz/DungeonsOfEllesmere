#ifndef DOOR_H
#define DOOR_H

#include "entity.h"
#include "player.h"

// class for in game doors
// currently only opening and unlocking supported, closing & locking not implemented
class Door {
    public:
        enum class State { kLocked, kClosed, kOpen };    
        enum class DoorType { kRegular, kSecret, kDiscovered };

        // constructs a door if size 2x1, composed of two entities "wing" and "anchor"
        Door(int x, int y, bool horizontal, bool secret, bool locked);
        
        // main interface
        void Interact(Player *player);        

        //getters
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