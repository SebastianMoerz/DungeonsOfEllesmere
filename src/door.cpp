#include "door.h"
#include <iostream>

Door::Door(int x, int y, bool horizontal, bool secret, bool locked) : _anchor(x,y,Entity::Type::kObstacle), _wing(x,y,Entity::Type::kObstacle), _horizontal(horizontal) {
           
    // set _wing position depending on door orientation
    if (horizontal) { _wing.SetPosition({x+1,y}); }
    if (!horizontal) { _wing.SetPosition({x,y+1}); }            

    if (locked) { _state = State::kLocked; }
    if (secret) { _type = DoorType::kSecret; }
};

void Door::Interact(Player *player) {
    if (_state == State::kOpen) { return; }

    std::cout << "---------------" << std::endl;

    if ( _type == DoorType::kSecret ) {       
        std::string isLocked = ( _state == State::kLocked ) ? "locked" : "unlocked";
        std::cout << "You found a secret door! It appears to be " << isLocked << "." << std::endl;
        _type = DoorType::kDiscovered;
        return;
    }

    if ( _state == State::kLocked ) {
                
        if (player->hasKey()) {
            std::cout << "You used the key to unlock the door." << std::endl;
            _state = State::kClosed;
        }
        else {
            std::cout << "The door is locked. Without a key there is no way to open it." << std::endl;
        }
        return;
    }

    if ( _state == State::kClosed) {
        std::cout << "You use all your weight to move the rusted hinges. Slowly the door creaks open." << std::endl;
        OpenDoor();
        _state = State::kOpen;
         return;
    }
}

void Door::OpenDoor() {

    int x = _anchor.GetPosition().x;
    int y = _anchor.GetPosition().y;

    if (_horizontal) {
        _anchor.SetPosition({x-1,y-1});
        _wing.SetPosition({x-1,y-2});            
    }
    else {
        _anchor.SetPosition({x+1,y-1});
        _wing.SetPosition({x+2,y-1});   
    }
};