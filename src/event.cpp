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

void MapEvent::Interact(Player *player) {
    std::cout << "---------------" << std::endl;
    std::cout << _msg << std::endl;

    if (_xp != 0) { player->ReceiveXP(_xp); } // std::cout << "You receive " << _xp << " experience points." << std::endl; }
    if (_dmg < 0) { player->Heal(_dmg); ; std::cout << "You heal " << _dmg << " hit points." << std::endl; }
    if (_dmg > 0) { player->TakeDamage (_dmg); std::cout << "You take " << _dmg << " points of damage." << std::endl;}

    this->MarkForErasure();
}       