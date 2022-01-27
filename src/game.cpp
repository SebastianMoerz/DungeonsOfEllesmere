#include <algorithm>
#include <iostream>

#include "game.h"
#include "game_utils.h"
#include "SDL.h"


Game::Game(std::size_t grid_width, std::size_t grid_height, std::size_t grid_margin, std::size_t num_opponents, std::size_t startdist)
    : _grid_max_x(grid_width), _grid_max_y(grid_height), engine(dev()), _NumberOfOpponents(num_opponents), _MinStartDistance(startdist),
      random_w(grid_margin, static_cast<int>(grid_width - grid_margin - 1)),random_h(grid_margin, static_cast<int>(grid_height - grid_margin - 1)) {


  

  std::cout << _wall.size() << std::endl;
  _obstaclemap = GameUtils::InitObstacleMap(_rendermap);

  _vicinitymap = GameUtils::GetVicinityMap();
  /*
  _tilesToNorth = GameUtils::SetExplorationArea(Entity::Direction::kUp, _vicinitymap);
  _tilesToSouth = GameUtils::SetExplorationArea(Entity::Direction::kDown, _vicinitymap);
  _tilesToEast = GameUtils::SetExplorationArea(Entity::Direction::kLeft, _vicinitymap);
  _tilesToWest = GameUtils::SetExplorationArea(Entity::Direction::kRight, _vicinitymap);

  */
 
  SetUpPlayer();
  SetUpGameMap(grid_height, grid_width, grid_margin);
  //PlaceTreasure();
  PlaceNPCs();
  //PlaceOpponents();
  //PlaceDoors();
  //PlaceEvents(); 
  //WelcomeMessage();
}


void Game::Run(Controller const &controller, Renderer &renderer, std::size_t target_frame_duration) {
  Uint32 title_timestamp = SDL_GetTicks();
  Uint32 frame_start;
  Uint32 frame_end;
  Uint32 frame_duration;
  int frame_count = 0;
  bool running = true;

  while (running) {
    frame_start = SDL_GetTicks();

    // Input, Update, Render - the main game loop.
    controller.HandleInput(running, _paused, _player);
    if (!_paused && !_won && _player.alive) {
      Update();
    }

    // without fog of war
    // renderer.DebugRender(_player, _treasure, _wall, _doors, _opponents, _npcs);
    // with fog of war (WIP)
    renderer.Render(_player, _treasure, _wall, _doors, _opponents, _npcs, _vicinitymap, _rendermap);

    frame_end = SDL_GetTicks();

    // Keep track of how long each loop through the input/update/render cycle takes.
    frame_count++;
    frame_duration = frame_end - frame_start;

    // After every second, update the window title.
    if (frame_end - title_timestamp >= 1000) {
      renderer.UpdateWindowTitle(frame_count);
      frame_count = 0;
      title_timestamp = frame_end;
    }

    // If the time for this frame is too small (i.e. frame_duration is smaller than the target ms_per_frame), delay the loop to achieve the correct frame rate.
    if (frame_duration < target_frame_duration) {
      SDL_Delay(target_frame_duration - frame_duration);
    }
  }
}


void Game::Update() {  
  if (!_player.alive) return;
  
  // UPDATE PLAYER
  // if user input event in queue, try to move player
  if (_player.direction != Player::Direction::kNone && _player.isMyTurnToMove()) {
    
    // calculate the position to which the player wants to move
    SDL_Point requestedPosition = _player.tryMove();
    
    // init path blocked and check for collisions:
    _pathBlocked = DetectCollision(requestedPosition, _wall);

    if (!IsOnMap(requestedPosition)) { _pathBlocked = true; };

    {
      Opponent* opponent = DetectCollision(requestedPosition, _opponents);
      if (opponent) {
        // kill player if collision with opponent occured        
        _pathBlocked = true;
        if (_player.isMyTurnToAttack()) { HandleFight (&_player, opponent); }               
        if (!opponent->alive) {
          std::unique_ptr<InventoryItem> loot = opponent->DropLoot();
          if (loot) {
            _treasure.emplace_back(std::make_unique<InteractiveE>(opponent->GetPosition().x, opponent->GetPosition().y, Entity::Type::kLoot, "You loot the body of your fallen opponent.\n"));
            _treasure.back()->AddItem(std::move(loot));
          }
          _player.ReceiveXP(opponent->GetXPValue());
          opponent->MarkForErasure();
        }
      }    
    }

    if (DetectCollisionAndInteract(&_player, requestedPosition, _treasure)) { _pathBlocked = true; }    
    if (DetectCollisionAndInteract(&_player, requestedPosition, _npcs)) { _pathBlocked = true; }
    if (DetectCollision(requestedPosition, _doors)) { 
      Door* door = DetectCollision(requestedPosition, _doors);
      door->Interact(&_player);
      _pathBlocked = true; 
    };
  
    // update position if movement is not blocked by obstacle
    if (!_pathBlocked) {_player.SetPosition(requestedPosition);}

    // check for events at the new position
    TriggerMapEvents(&_player, _events);
  }

  // UPDATE OPPONENTS
  // only try to move if it's the opponents turn to avoid unnecessary checkCollision loops.
  for (std::unique_ptr<Opponent> &opponent : _opponents) {
    if (opponent->isMyTurnToMove()) {
      // calculate the position to which the opponent wants to move
      SDL_Point requestedPosition = opponent->tryMove(GameUtils::MoveTowardTarget(GetMapOfObstacles(), opponent->GetPosition(), _player.GetPosition(), 30), _player.GetPosition());
      //SDL_Point requestedPosition = opponent->tryMove();
        
    // init path blocked and check for collisions:
    _pathBlocked = DetectCollision(requestedPosition, _wall);

    if (!IsOnMap(requestedPosition)) { _pathBlocked = true; };

      if (DetectCollision(requestedPosition, _opponents)) { _pathBlocked = true; };
      if (DetectCollision(requestedPosition, _doors)) { _pathBlocked = true; };

      if (DetectCollision(requestedPosition, &_player)) {
        // kill player if collision with opponent occured
        if (opponent->isMyTurnToAttack()) { HandleFight(opponent.get(), &_player); }
        _pathBlocked = true;
      }    
      if (DetectCollision(requestedPosition, _treasure)) { _pathBlocked = true; }
      if (DetectCollision(requestedPosition, _npcs)) { _pathBlocked = true; }

      // update position if movement is not blocked by obstacle
      if (!_pathBlocked) {opponent->SetPosition(requestedPosition);}
    }   
  }  
  
  // "Game Over" message  
  if (!_player.alive) { 
    std::cout << "---------------" << std::endl; 
    std::cout << "GAME OVER: You were killed!" << std::endl;
    std::cout << "---------------" << std::endl; 
  }
  if (_player.GetQuestComplete()) { 
    _won = true;
    std::cout << "---------------" << std::endl; 
    std::cout << "YOU WON!" << std::endl;
    std::cout << "---------------" << std::endl; 
  }
  
  CleanUpErasedEntities();
}

// check if a position is on the game map
bool Game::IsOnMap(SDL_Point point) {  
  if ( point.x < 0 ) { return false; }
  if ( point.y < 0 ) { return false; }
  if ( point.x >= _grid_max_x ) { return false; }
  if ( point.y >= _grid_max_y ) { return false; }
  return true;
}

// Collision Detection #1: Collision between Point and Entity (e.g. for collisions between opponent and player)
bool Game::DetectCollision(SDL_Point point, Entity *entity) {
  if (entity != nullptr) {
    return (point.x == entity->GetPosition().x && point.y == entity->GetPosition().y);
  }
  return false;
}

// Collision Detection #2: Collision between Point and Vector of Entities (via unique ptr). Used to check if movement to Point is valid e.g. for movement or for initial placement of objects
bool Game::DetectCollision(SDL_Point point, std::vector<std::unique_ptr<Entity>> &entities) {
  if (!entities.empty()) {
    auto it = std::find_if(entities.begin(), entities.end(), [&point](const std::unique_ptr<Entity>& item){return point.x == item->GetPosition().x && point.y == item->GetPosition().y;});
    if (it != entities.end()) { return true; }  
  }  
  return false;
}

// Collision Detection #3: Collision with Interactive Entities without invoking interaction method
bool Game::DetectCollision(SDL_Point point, std::vector<std::unique_ptr<InteractiveE>> &entities) {
  if (!entities.empty()) {
    auto it = std::find_if(entities.begin(), entities.end(), [&point](const std::unique_ptr<InteractiveE>& item){return point.x == item->GetPosition().x && point.y == item->GetPosition().y;});
    if (it != entities.end()) {      
      if ((*it)->GetBlocksPath()) { return true; }
    }
  }
  return false;
}

// Collision Detection #4: Collision between Point and Vector of Opponents (via unique ptr). Used e.g. to get the opponent handle if the player attacks.
Opponent* Game::DetectCollision(SDL_Point point, std::vector<std::unique_ptr<Opponent>> &opponents) {
  if (!opponents.empty()) {
    auto it = std::find_if(opponents.begin(), opponents.end(), [&point](const std::unique_ptr<Opponent>& item){return point.x == item->GetPosition().x && point.y == item->GetPosition().y;});
    if (it != opponents.end()) { return (*it).get(); }  
  }  
  return nullptr;
}

// Collision Detection #5: Collision with doors
Door* Game::DetectCollision(SDL_Point point, std::vector<std::unique_ptr<Door>> &doors) {
  if (!doors.empty()) {
    auto it = std::find_if(doors.begin(), doors.end(), [&point](const std::unique_ptr<Door>& item){
      return (point.x == item->GetAnchorPosition().x && point.y == item->GetAnchorPosition().y) ||(point.x == item->GetWingPosition().x && point.y == item->GetWingPosition().y);
     });
    if (it != doors.end()) { return (*it).get(); }  
  }  
  return nullptr;
}

// Collision Detection #6: Collision between Player and Interactive Entities - calls InteractiveE::Interact() method upon collision
bool Game::DetectCollisionAndInteract(Player *player, SDL_Point point, std::vector<std::unique_ptr<InteractiveE>> &entities) {
  if (!entities.empty()) {
    auto it = std::find_if(entities.begin(), entities.end(), [&point](const std::unique_ptr<InteractiveE>& item){return point.x == item->GetPosition().x && point.y == item->GetPosition().y;});
    if (it != entities.end()) {      
      (*it)->Interact(player);
      if ((*it)->GetBlocksPath()) { return true; }
    }
  }
  return false;
}

// basically another collision method
void Game::TriggerMapEvents(Player *player, std::vector<std::unique_ptr<MapEvent>> &events) {
  for (std::unique_ptr<MapEvent> &event : events) {
    if (event->isInArea(player->GetPosition().x, player->GetPosition().y)) { event->Interact(player); }
  }
}

// get a representation of the game map where each element is either "kNone" or "kObstacle"
// used as input for calculating the opponent's movement
std::vector<std::vector<Entity::Type>> Game::GetMapOfObstacles () {
  std::vector<std::vector<Entity::Type>> grid {};
  std::vector<Entity::Type> row {};
  
  // init emtpy grid (of format grid[y][x])
  for (int y = 0; y < _grid_max_y; y++) {
    row.clear();
    for (int x = 0; x < _grid_max_x; x++) {
      row.push_back(Entity::Type::kNone);
    }
    grid.push_back(row);
  }  

  // add obstacles to grid
  for (std::unique_ptr<Entity> &brick : _wall) {
    grid[brick->GetPosition().y][brick->GetPosition().x] = Entity::Type::kObstacle;    
  }
  for (std::unique_ptr<InteractiveE> &npc : _npcs) {
    grid[npc->GetPosition().y][npc->GetPosition().x] = Entity::Type::kObstacle;    
  }
  for (std::unique_ptr<Door> &door : _doors) {
    grid[door->GetAnchorPosition().y][door->GetAnchorPosition().x] = Entity::Type::kObstacle;
    grid[door->GetWingPosition().y][door->GetWingPosition().x] = Entity::Type::kObstacle;
  }
  return grid;
}


void Game::CleanUpErasedEntities() {    
  // the following should eventually be done with a template function.
  // check wall (necessary for door placement)
  while (true) {
    auto it = std::find_if(_wall.begin(), _wall.end(), [](const std::unique_ptr<Entity>& item){return item->isMarkedForErasure();});
    if (it == _wall.end()) {       
      break;
    }
    else {
      _wall.erase(it);
    }
  }
  // check opponent
  while (true) {
   auto it = std::find_if(_opponents.begin(), _opponents.end(), [](const std::unique_ptr<Opponent>& item){return item->isMarkedForErasure();});
   if (it == _opponents.end()) {       
     break;
   }
   else {
    _opponents.erase(it);
   }
  }
  // check treasure
  while (true) {
    auto it = std::find_if(_treasure.begin(), _treasure.end(), [](const std::unique_ptr<InteractiveE>& item){return item->isMarkedForErasure();});
    if (it == _treasure.end()) {       
      break;
    }
    else {
      _treasure.erase(it);
    }
  }
  // check events
  while (true) {
    auto it = std::find_if(_events.begin(), _events.end(), [](const std::unique_ptr<MapEvent>& item){return item->isMarkedForErasure();});
    if (it == _events.end()) {       
      break;
    }
    else {
      _events.erase(it);
    }
  }
  // currently no erasable NPCs yet
}

// deal damage to defender
void Game::HandleFight (Combattant* attacker, Combattant* defender) {
  std::cout << "---------------" << std::endl; 

  // who attacks' who?
  if ( attacker == &_player ) { 
    std::cout << "You attack " << defender->GetName(); 
  } else if ( defender == &_player ) {
    std::cout << attacker->GetName() << " attacks you"; 
  } else {
    std::cout << attacker->GetName() << " attacks " << defender->GetName();
  }

  // get damage
  int damage = (rand() % attacker->GetAttackValue()) - (rand() % defender->GetDefenseValue());

  // if nobody got hurt 
  if (damage <= 0) { 
    if ( attacker == &_player ) { std::cout << " and miss." << std::endl; }
    else { std::cout << " and misses." << std::endl; }      
  } 
  else {
    // deal damage
    defender->TakeDamage(damage);

    // print status message       
    if ( defender == &_player ) {
      std::cout << " and hits you for " << damage << " points of damage."<< std::endl; 
      std::cout << "Hit points remaining: (" << defender->GetHP() << "/" << defender->GetMaxHP() << ")" << std::endl;
    }          
    else { 
        std::string s = ( attacker == &_player ) ? "" : "s";
        std::cout << " and hit" << s << " for " << damage << " points of damage." << std::endl; 

        float health = static_cast<float>(defender->GetHP()) / static_cast<float>(defender->GetMaxHP());
        if (health >= 0.8) {std::cout << defender->GetName() << " is bruised." << std::endl; } 
        else if (health <= 0.0 ) {std::cout << defender->GetName() << " is dead." << std::endl; }   
        else if (health <= 0.15) {std::cout << defender->GetName() << " is nearly dead." << std::endl; }          
        else if (health <= 0.3 ) {std::cout << defender->GetName() << " is heavily wounded." << std::endl; }
        else if (health <= 0.5) {std::cout << defender->GetName() << " is wounded." << std::endl; }
    }
  }
}

// eventually all the methods below should be read from file to allow for different game maps/dungeon levels
void Game::SetUpPlayer() {  
  _player.SetPosition({10,37});
  // SET UP PLAYER INVENTORY
  {
    std::unique_ptr<InventoryItem> item = std::make_unique<InventoryItem>();
    item->name = "Rusty Sword";
    item->number = 1;
    item->attack_mod = 2;
    item->isWeapon = true;    
    _player.receiveItem(std::move(item), true);
    _player.SelectItem(1, true);
  }
  {
    std::unique_ptr<InventoryItem> item = std::make_unique<InventoryItem>();
    item->name = "Moldy Bread";
    item->number = 1;
    item->healing = 2;
    item->isSingleUseItem = true;    
    _player.receiveItem(std::move(item), true);    
  }
}

void Game::SetUpGameMap(int grid_height, int grid_width, int grid_margin) {
  /*
  // uncomment to paint unbroken walls around screen    
  for (std::size_t a = grid_margin; a < grid_width-grid_margin; a++) {  
      _wall.emplace_back(std::make_unique<Entity>(a, grid_margin, Entity::Type::kObstacle));                 // upper wall      
      _wall.emplace_back(std::make_unique<Entity>(a, grid_height-grid_margin-1, Entity::Type::kObstacle));   // lower wall      
  }
  for (std::size_t a = grid_margin; a < grid_height-grid_margin; a++) {
      _wall.emplace_back(std::make_unique<Entity>(grid_margin, a, Entity::Type::kObstacle));                 // left wall
      _wall.emplace_back(std::make_unique<Entity>(grid_width-grid_margin-1, a, Entity::Type::kObstacle));    // right wall
  }
  */  


  /* hardcoded PoC map

  int lower_margin = 4;
  for (std::size_t a = 0; a < grid_width; a++) {  
        _wall.emplace_back(std::make_unique<Entity>(a, 0, Entity::Type::kObstacle));     // upper wall      
        switch (a) {
          case 6 ... 10 : { break; }  // leave entrance to cave open
          default : 
            _wall.emplace_back(std::make_unique<Entity>(a, grid_height-lower_margin-1, Entity::Type::kObstacle));   // lower wall - leave some space to simulate the outside of the cave
        }      
    }
    for (std::size_t a = 0; a < grid_height-lower_margin; a++) {
        _wall.emplace_back(std::make_unique<Entity>(0, a, Entity::Type::kObstacle));             // left wall
        _wall.emplace_back(std::make_unique<Entity>(grid_width, a, Entity::Type::kObstacle));    // right wall
    }
    // inner walls
    // horizontal walls
    for (std::size_t a = 0; a < 38; a++) { _wall.emplace_back(std::make_unique<Entity>(a, 35, Entity::Type::kObstacle)); }
    for (std::size_t a = 40; a < grid_width; a++) { _wall.emplace_back(std::make_unique<Entity>(a, 35,  Entity::Type::kObstacle)); }
    for (std::size_t a = 0; a < 25; a++) { _wall.emplace_back(std::make_unique<Entity>(a, 23, Entity::Type::kObstacle)); }
    for (std::size_t a = 34; a < 41; a++) { _wall.emplace_back(std::make_unique<Entity>(a, 10, Entity::Type::kObstacle)); }
    for (std::size_t a = 34; a < 38; a++) { _wall.emplace_back(std::make_unique<Entity>(a, 28, Entity::Type::kObstacle)); }

    //vertical walls
    for (std::size_t a = 4; a < 24; a++) { _wall.emplace_back(std::make_unique<Entity>(25, a, Entity::Type::kObstacle)); }
    _wall.emplace_back(std::make_unique<Entity>(25, 1, Entity::Type::kObstacle));
    for (std::size_t a = 23; a < 30; a++) { _wall.emplace_back(std::make_unique<Entity>(20, a, Entity::Type::kObstacle)); }
    for (std::size_t a = 28; a < 36; a++) { _wall.emplace_back(std::make_unique<Entity>(12, a, Entity::Type::kObstacle)); }
    for (std::size_t a = 10; a < 28; a++) { _wall.emplace_back(std::make_unique<Entity>(34, a, Entity::Type::kObstacle)); }
    for (std::size_t a = 0; a < 11; a++) { _wall.emplace_back(std::make_unique<Entity>(40, a, Entity::Type::kObstacle)); }

    //pillars
    _wall.emplace_back(std::make_unique<Entity>(42, 28, Entity::Type::kObstacle));
    _wall.emplace_back(std::make_unique<Entity>(45, 28, Entity::Type::kObstacle));
    _wall.emplace_back(std::make_unique<Entity>(46, 28, Entity::Type::kObstacle));
    _wall.emplace_back(std::make_unique<Entity>(44, 17, Entity::Type::kObstacle));
    _wall.emplace_back(std::make_unique<Entity>(38, 23, Entity::Type::kObstacle));
    
    */

    _rendermap = GameUtils::GetRenderBaseMap("../src/levelmap.txt");
    _wall = GameUtils::GetWallFromMap(_rendermap);
}
  
void Game::PlaceTreasure() {
  {
    std::unique_ptr<InventoryItem> somecoins = std::make_unique<InventoryItem>();
    somecoins->name = "Gold Coin";
    somecoins->number = 3;
    std::string msg = "You found some golden coins scattered across the dirty floor.\n";

    _treasure.emplace_back(std::make_unique<InteractiveE>(10, 10, Entity::Type::kTreasure, msg));
    _treasure.back()->AddItem(std::move(somecoins));
  }
  {
    std::unique_ptr<InventoryItem> morecoins = std::make_unique<InventoryItem>();
    morecoins->name = "Gold Coin";
    morecoins->number = 5;
    std::string msg = "You found some golden coins scattered across the dirty floor.\n";

    _treasure.emplace_back(std::make_unique<InteractiveE>(2, 34, Entity::Type::kTreasure,  msg));
    _treasure.back()->AddItem(std::move(morecoins));
  }
  {
    std::unique_ptr<InventoryItem> morecoins = std::make_unique<InventoryItem>();
    morecoins->name = "Gold Coin";
    morecoins->number = 2;
    std::string msg = "You found some golden coins scattered across the dirty floor.\n";

    _treasure.emplace_back(std::make_unique<InteractiveE>(4, 41, Entity::Type::kTreasure,  msg));
    _treasure.back()->AddItem(std::move(morecoins));
  }
  {
    std::unique_ptr<InventoryItem> key = std::make_unique<InventoryItem>();
    key->name = "Old Bronze Key";
    key->number = 1;
    key->isKey = true;
    std::string msg = "Hidden beneath the rubble you find an old key made out of patinized bronze.\n";

    _treasure.emplace_back(std::make_unique<InteractiveE>(2, 44, Entity::Type::kTreasure,  msg));
    _treasure.back()->AddItem(std::move(key));
  }

   {
    std::unique_ptr<InventoryItem> item = std::make_unique<InventoryItem>();
    item->name = "Strange-looking Potion";
    item->number = 1;
    item->healing = 6;
    item->isSingleUseItem = true;  
    std::string msg = "A skeleton lies slouched against the back of the cave. Stalagmites begin to grow on it's ancient bones. \nClutched between it's fingers is a small flacon made of clouded cristal. The reddish liquid inside seems to give of a faint glow.\n"; 

    _treasure.emplace_back(std::make_unique<InteractiveE>(39, 9, Entity::Type::kTreasure,  msg));
    _treasure.back()->AddItem(std::move(item)); 
  }

  {
    std::unique_ptr<InventoryItem> mcguffin = std::make_unique<InventoryItem>();
    mcguffin->name = "McGuffin";
    mcguffin->number = 1;    
    mcguffin->isMcGuffin = true;
    std::string msg = "You found the legendary Golden McGuffin of King Lazyplot VIII!\n";

    _treasure.emplace_back(std::make_unique<InteractiveE>(5, 20, Entity::Type::kTreasure,  msg));
    _treasure.back()->AddItem(std::move(mcguffin));    
  }
  {
    std::unique_ptr<InventoryItem> sword = std::make_unique<InventoryItem>();
    sword->name = "Magic Blade";
    sword->number = 1;
    sword->attack_mod = 4;
    sword->isWeapon = true;  
    std::unique_ptr<InventoryItem> armor = std::make_unique<InventoryItem>();
    armor->name = "Sturdy Hauberk";
    armor->number = 1;
    armor->attack_mod = 3;
    armor->isArmor = true;
    std::string msg = "You find an old wooden chest, covered in cobwebs. You open the lid carefully.\n";

    _treasure.emplace_back(std::make_unique<InteractiveE>(45, 3, Entity::Type::kChest, msg));
    _treasure.back()->AddItem(std::move(sword));
    _treasure.back()->AddItem(std::move(armor));
  }
}
  
void Game::PlaceNPCs() {
  _npcs.emplace_back(std::make_unique<InteractiveE>(12, 35, "../src/dialogue.txt"));
  _npcs.at(0)->SetAsMainQuestGiver();
}

void Game::PlaceOpponents() {
  int x, y;
  SDL_Point point;
  for (int i = 0; i < _NumberOfOpponents; i++) {
    while (true) {
      x = random_w(engine);
      y = random_h(engine);
      point = {x,y};
      // Check if point is not already occupied and not to close to initial player position
      if (!DetectCollision(point, _wall) && DetectCollision(point, _opponents) == nullptr && GameUtils::Heuristic(x, y, _player.GetPosition().x, _player.GetPosition().y) >= _MinStartDistance && y < 34) {
        _opponents.emplace_back(std::make_unique<Opponent>(x, y, Entity::Type::kNPC));      
        break;
      }
    }  
  }
}
 

void Game::WelcomeMessage() {
  std::cout << std::endl << "-----------------------------------------------------" << std::endl;
  std::cout << "DUNGEONS OF ELLESMERE - QUEST FOR THE GOLDEN McGUFFIN" << std::endl;
  std::cout << "-----------------------------------------------------" << std::endl << std::endl;
  std::cout << "Welcome to Dungeons of Ellesmere, a proof-of-concept RPG written in C++ using the SDL-Library." << std::endl;
  std::cout << "You'll play the role of a generic adventurer searching for the legendary McGuffin of King Lazyplot." << std::endl << std::endl;
  std::cout << "Use the arrow keys to move the adventurer (blue square) across the map." << std::endl;
  std::cout << "Move on in-game objects to interact with them. You can:" << std::endl;
  std::cout << "- talk to friendly NPCs (light blue squares)" << std::endl;
  std::cout << "- collect treasure (yellow squares)" << std::endl;
  std::cout << "- open treasure chests (brown squares)" << std::endl;
  std::cout << "- fight opponents (red squares)" << std::endl << std::endl;
  std::cout << "In order to fight an opponent, you have to repeatedly move toward it using the arrow keys. Each key press equals one strike." << std::endl << std::endl;
  std::cout << "Status information and dialogue is printed on the console, so make sure to keep an eye on the text output as well as the game map!" << std::endl << std::endl;
  std::cout << "Game control:" << std::endl;
  std::cout << "At any time during the game, you can:" << std::endl;
  std::cout << "Press (p) for pausing / unpausing the game" << std::endl;
  std::cout << "Press (i) to take a look at your inventory" << std::endl;
  std::cout << "Press (c) to check your adventurer's health" << std::endl;
  std::cout << "Press (1-9) to use or equip items from your inventory" << std::endl << std::endl;
  std::cout << "Now have fun and save the world!" << std::endl;
}

void Game::PlaceDoors() {
  // DEMO DOORS
  _doors.emplace_back(std::make_unique<Door>(38, 35, true, false, true));
  //_doors.emplace_back(std::make_unique<Door>(34, 40, false, false, true));
  //_doors.emplace_back(std::make_unique<Door>(25, 40, false, true, true));
  
}

void Game::PlaceEvents() {
  _events.emplace_back(std::make_unique<MapEvent>(6,43,"The air smells like mold."));
  _events.back()->AddToArea(7,43);
  _events.back()->AddToArea(8,43);
  _events.back()->AddToArea(9,43);
  _events.back()->AddToArea(10,43);
}