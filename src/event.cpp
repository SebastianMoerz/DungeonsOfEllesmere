#include "event.h"


void MapEvent::AddToArea(int x, int y) {
    if (!isInArea(x,y)) { _area.emplace_back(std::make_unique<Entity>(x,y,Entity::Type::kEvent)); }
}
        
void MapEvent::RemoveFromArea(int x, int y) {
    while (true) {
        auto it = std::find_if(_area.begin(), _area.end(), [&](const std::unique_ptr<Entity>& item) { return (item->GetPosition().x == x && item->GetPosition().y == y);});
        if (it == _area.end()) { return; }
        else { _area.erase(it); }
    }
}

bool MapEvent::isInArea(int x, int y) {
    for (std::unique_ptr<Entity> &entity : _area) {
        if (entity->GetPosition().x == x && entity->GetPosition().y == y) { return true; }
    }
    return false;
}

std::vector<SDL_Point> MapEvent::GetArea() {
    std::vector<SDL_Point> positions{};
    for (std::unique_ptr<Entity> &tile : _area) {
        positions.push_back(tile->GetPosition());
    }
   return positions;
}

void MapEvent::Interact(Player *player) {

    // check if event is side quest
    if (_type == EventType::kCollectionQuest) {
        RemoveFromArea(player->GetPosition().x, player->GetPosition().y);
        // check if all tiles belonging to the quest were visited
        if (_area.empty()) {
            
            if (_msg != "") { std::cout << "---------------" << std::endl << _msg << std::endl;}
            if (_xp != 0) { player->ReceiveXP(_xp); } 
            if (_dmg < 0) { player->Heal(_dmg); ; std::cout << "You heal " << -(_dmg) << " hit points." << std::endl; }
            if (_dmg > 0) { player->TakeDamage (_dmg); std::cout << "You take " << _dmg << " points of damage." << std::endl;}
            
            // delete event if quest is completed
            this->MarkForErasure();
        }
        return;
    }

    if (_type == EventType::kIllumination) {
        if (_msg != "") { std::cout << "---------------" << std::endl << _msg << std::endl;}
        player->SetVision(_illumination);
        return;
    }

    // for kSingle and kPersistent
    std::cout << "---------------" << std::endl << _msg << std::endl;
    if (_xp != 0) { player->ReceiveXP(_xp); } // std::cout << "You receive " << _xp << " experience points." << std::endl; }
    if (_dmg < 0) { player->Heal(_dmg); ; std::cout << "You heal " << -(_dmg) << " hit points." << std::endl; }
    if (_dmg > 0) { player->TakeDamage (_dmg); std::cout << "You take " << _dmg << " points of damage." << std::endl;}

    // delete event from game if type is kSingle
    if (_type == EventType::kSingle ) { this->MarkForErasure(); }
}       
