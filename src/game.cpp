#include <algorithm>
#include <iostream>

#include "game.h"
#include "game_utils.h"
#include "SDL.h"


// -----------------
// SETTING UP THE GAME
// -----------------

Game::Game(std::size_t grid_width, std::size_t grid_height) : _grid_max_x(grid_width), _grid_max_y(grid_height), engine(dev()), 
      random_w(0, static_cast<int>(grid_width - 1)),random_h(0, static_cast<int>(grid_height - 1)) {
  
  SetUpPlayer(10,37);  
  SetUpGameMap("../src/levelmap.txt");
  PlaceTreasure();
  PlaceNPCs();
  PlaceOpponents(); 
  PlaceDoors();
  PlaceEvents();
  WelcomeMessage();
}

// ----------
// GAME LOOP
// ----------

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

    // with fog of war (WIP)
    renderer.Render(_player, _treasure, _wall, _doors, _opponents, _npcs, _vicinitymap, _rendermap);
    // without fog of war (bool = true if screen shall be cleared (default) - set to false if used on top of regular rendering)
    //renderer.DebugRender(_player, _treasure, _wall, _doors, _opponents, _npcs, _events, true);

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


// ------------------------
// THE CENTRAL GAME METHOD
// ------------------------

void Game::Update() {  
  if (!_player.alive) return;
  
  // check for timed effect triggers
  _player.UpdateEffects();

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
      SDL_Point requestedPosition = opponent->tryMove(GameUtils::MoveTowardTarget(GetMapOfObstacles(), opponent->GetPosition(), _player.GetPosition(), 20), _player.GetPosition());
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


// --------------------------------------------------------------------------------------------------------
// VARIOUS COLLISION DETECTION METHODS - SOME OF WHICH COULD BE MADE OBSOLETE IF _OBSTACLEMAP IS BEING USED
// --------------------------------------------------------------------------------------------------------

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


// -------------------
// GAME WORLD CONTROL
// -------------------

// get a representation of the game map where each element is either "kNone" or "kObstacle"
// used as input for calculating the opponent's movement
// CAN BE MADE OBSOLETE IF _OBSTACLEMAP IS BEING USED
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

// -----------------
// COMBAT FUNCTIONS
// -----------------

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
        std::cout << " and hit" << s << " for " << damage << " points of damage." << std::endl; //
  
        float health = static_cast<float>(defender->GetHP()) / static_cast<float>(defender->GetMaxHP());
        if (health >= 0.8) {std::cout << defender->GetName() << " is bruised." << std::endl; } 
        else if (health <= 0.0 ) {std::cout << defender->GetName() << " is dead." << std::endl; }   
        else if (health <= 0.15) {std::cout << defender->GetName() << " is nearly dead." << std::endl; }          
        else if (health <= 0.3 ) {std::cout << defender->GetName() << " is heavily wounded." << std::endl; }
        else if (health <= 0.5) {std::cout << defender->GetName() << " is wounded." << std::endl; }
    }
  }
}
  

// -----------------
// HELPER FUNCTIONS
// -----------------

// check if a position is on the game map
bool Game::IsOnMap(SDL_Point point) {  
  if ( point.x < 0 ) { return false; }
  if ( point.y < 0 ) { return false; }
  if ( point.x >= _grid_max_x ) { return false; }
  if ( point.y >= _grid_max_y ) { return false; }
  return true;
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



// ----------------------------------------------------------------------------
// SETTING UP THE GAME WORLD: READ MAP, INIT PLAYER, PLACE NPCs, TREASURE, etc.
// ----------------------------------------------------------------------------


// read game map from file
void Game::SetUpGameMap(std::string filepath) {
 
    // read game map from file & store tile type information for rendering
    _rendermap = GameUtils::GetRenderBaseMap(filepath);
    
    // init the (hardcoded) vicinity map (for rendering)
    _vicinitymap = GameUtils::GetVicinityMap();
    
    // init walls for collision detection - can eventually be replaced by _obstaclemap
    _wall = GameUtils::GetWallFromMap(_rendermap);
    _obstaclemap = GameUtils::InitObstacleMap(_rendermap); // currently not used yet - also, GameUtils::InitObstacleMap still WIP
    
}

// eventually all the methods below should be read from file to allow for different game maps/dungeon levels
void Game::SetUpPlayer(int x, int y) {  
  _player.SetPosition({x,y});
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
  {
    std::unique_ptr<InventoryItem> item = std::make_unique<InventoryItem>();
    item->name = "Torch";
    item->number = 1;    
    item->isSingleUseItem = true;    
    _player.receiveItem(std::move(item), true);    
  }
}
 
  
void Game::PlaceNPCs() {
  _npcs.emplace_back(std::make_unique<InteractiveE>(12, 35, "../src/dialogue.txt"));
  _npcs.at(0)->SetAsMainQuestGiver();
}

void Game::PlaceDoors() {  
  _doors.emplace_back(std::make_unique<Door>(11, 17, true, false, true));  
  _doors.emplace_back(std::make_unique<Door>(30, 5, true, true, false));
  
}

void Game::PlaceOpponents() {

  //_opponents.emplace_back(std::make_unique<Opponent>(5, 8, Entity::Type::kNPC));
  _opponents.emplace_back(std::make_unique<Opponent>(7, 4, Entity::Type::kNPC));
  _opponents.emplace_back(std::make_unique<Opponent>(11, 8, Entity::Type::kNPC));
  _opponents.emplace_back(std::make_unique<Opponent>(14, 6, Entity::Type::kNPC));
  _opponents.emplace_back(std::make_unique<Opponent>(29, 21, Entity::Type::kNPC));
}


void Game::PlaceEvents() {
  // enter cave
  _events.emplace_back(std::make_unique<MapEvent>(8, 31, "It is dark in here. The air inside smells like mold."));
  _events.back()->AddToArea(9,31);
  _events.back()->AddToArea(10,31);
  _events.back()->AddToArea(11,31);  

    // dim lights on entering cave
    _events.emplace_back(std::make_unique<MapEvent>(8, 31, Player::Vision::kCavern, ""));
    _events.back()->AddToArea(9,31);
    _events.back()->AddToArea(10,31);
    _events.back()->AddToArea(11,31);  

    // increase brightness when leaving cave
    _events.emplace_back(std::make_unique<MapEvent>(8, 32, Player::Vision::kDaylight, ""));
    _events.back()->AddToArea(9,32);
    _events.back()->AddToArea(10,32);
    _events.back()->AddToArea(11,32);  


  // rock collection side quest
  _events.emplace_back(std::make_unique<MapEvent>(4,19, MapEvent::EventType::kCollectionQuest, "", 30, 0));
  _events.back()->AddToArea(28,1);
  _events.back()->AddToArea(43,19);

  // enter alcove main cavern
  _events.emplace_back(std::make_unique<MapEvent>(18,27,"This small alcove is covered almost entirely with stalagmites. Droplets of water drip steady from the ceiling, hidden in the darkness above."));
  
  // spot a lizard
  _events.emplace_back(std::make_unique<MapEvent>(26,14,"You spot a small lizard. It's eyes are milky-white.",1,0));

  // trap
  _events.emplace_back(std::make_unique<MapEvent>(34,6,"You trip and tumble against the wall. The vibrations break one of the stalagtites above. It crashes down before you even notice it.",0,5));
  _events.back()->AddToArea(34,7);
  _events.back()->AddToArea(35,7);
  _events.back()->AddToArea(33,6);
  _events.back()->AddToArea(33,7);
  _events.back()->AddToArea(33,8);
  _events.back()->AddToArea(34,8);
  _events.back()->AddToArea(35,8);

  // warn player of trap
  _events.emplace_back(std::make_unique<MapEvent>(32,6,"There are a lot of stalagtites hanging from the ceiling. Some of them seem rather fragile."));
  _events.back()->AddToArea(32,7);
  _events.back()->AddToArea(32,8);
  _events.back()->AddToArea(33,9);
  _events.back()->AddToArea(34,9);
  _events.back()->AddToArea(35,9);

  // light change
  // increase brightness when leaving corridor
  _events.emplace_back(std::make_unique<MapEvent>(32, 1, Player::Vision::kCavern, ""));
  // decrease brightness
  _events.emplace_back(std::make_unique<MapEvent>(33,1, Player::Vision::kDark1, ""));
  _events.emplace_back(std::make_unique<MapEvent>(33,1,"The corridor ahead is pitch black."));
  _events.emplace_back(std::make_unique<MapEvent>(34,1, Player::Vision::kDark2, ""));
  _events.emplace_back(std::make_unique<MapEvent>(34,1,"Maybe you should return?."));
  _events.emplace_back(std::make_unique<MapEvent>(35,1, Player::Vision::kDark3, ""));
  _events.emplace_back(std::make_unique<MapEvent>(35,1,"You can barely see your own hands."));


  // a spooky encounter
  _events.emplace_back(std::make_unique<MapEvent>(47,14,"As you venture deeper into the cave, you hear a strange sound ahead. It almost sounds like a something was moaning in the dark, but you're not quite sure."));
  _events.back()->AddToArea(48,14);
  _events.back()->AddToArea(49,14);
  _events.emplace_back(std::make_unique<MapEvent>(44,23,"You notice a faint blue reflection on the cave wall ahead of you. It seems almost unnatural."));
  _events.emplace_back(std::make_unique<MapEvent>(45,29,"Toward the back of the room an eerie apparition hovers in mid air. It's tanslucent skin shimmers in different hues of blue. Without lifting it's head, the figure begins to beckon you."));
}

 
void Game::PlaceTreasure() {
  // coins
  {
    std::unique_ptr<InventoryItem> item = std::make_unique<InventoryItem>();
    item->name = "Gold Coin";
    item->number = 3;
    std::string msg = "You found some golden coins scattered across the dirty floor.\n";

    _treasure.emplace_back(std::make_unique<InteractiveE>(4, 26, Entity::Type::kTreasure, msg));
    _treasure.back()->AddItem(std::move(item));
  }
  {
    std::unique_ptr<InventoryItem> item = std::make_unique<InventoryItem>();
    item->name = "Gold Coin";
    item->number = 1;
    std::string msg = "You find a single gold coin hidden in the rubble.\n";

    _treasure.emplace_back(std::make_unique<InteractiveE>(48, 20, Entity::Type::kTreasure,  msg));
    _treasure.back()->AddItem(std::move(item));
  }

  // add key
  {
    std::unique_ptr<InventoryItem> key = std::make_unique<InventoryItem>();
    key->name = "Old Bronze Key";
    key->number = 1;
    key->isKey = true;
    std::string msg = "A scuffed bronze key lies hidden beneath a pile of gnawed-off bones. You wonder how it ended up here.\n";

    _treasure.emplace_back(std::make_unique<InteractiveE>(34, 15, Entity::Type::kTreasure,  msg));
    _treasure.back()->AddItem(std::move(key));
  }

  // rocks
  {
    std::unique_ptr<InventoryItem> item = std::make_unique<InventoryItem>();
    item->name = "Rock";
    item->number = 1;
    std::string msg = "You notice a pebble lying in the dust. It appears to be an ordinary stone.\n";

    _treasure.emplace_back(std::make_unique<InteractiveE>(4, 19, Entity::Type::kTreasure, msg));
    _treasure.back()->AddItem(std::move(item));
  }
  {
    std::unique_ptr<InventoryItem> item = std::make_unique<InventoryItem>();
    item->name = "Rock";
    item->number = 1;
    std::string msg = "You glimpse a small piece of granite. A nice-looking quartz incursion runs along its surface.\n";

    _treasure.emplace_back(std::make_unique<InteractiveE>(28, 1, Entity::Type::kTreasure, msg));
    _treasure.back()->AddItem(std::move(item));
  }
  {
    std::unique_ptr<InventoryItem> item = std::make_unique<InventoryItem>();
    item->name = "Rock";
    item->number = 1;
    std::string msg = "You find a smooth little rock. It's almost unremarkable. Almost.\n";

    _treasure.emplace_back(std::make_unique<InteractiveE>(43, 19, Entity::Type::kTreasure, msg));
    _treasure.back()->AddItem(std::move(item));
  }

  // potions
  {
    std::unique_ptr<InventoryItem> item = std::make_unique<InventoryItem>();
    item->name = "Strange-looking Potion";
    item->number = 1;
    item->healing = 6;
    item->isSingleUseItem = true;  
    std::string msg = "A skeleton lies slouched against the back of the cave. Stalagmites begin to grow on it's ancient bones. \nClutched between it's fingers is a small flacon made of clouded cristal. The reddish liquid inside seems to give of a faint glow.\n"; 

    _treasure.emplace_back(std::make_unique<InteractiveE>(20, 29, Entity::Type::kTreasure,  msg));
    _treasure.back()->AddItem(std::move(item)); 
  }

  // treasure chests
  {
    std::unique_ptr<InventoryItem> coins = std::make_unique<InventoryItem>();
    coins->name = "Gold Coin";
    coins->number = 10;
    std::unique_ptr<InventoryItem> armor = std::make_unique<InventoryItem>();
    armor->name = "Sturdy Hauberk";
    armor->number = 1;
    armor->attack_mod = 3;
    armor->isArmor = true;
    std::string msg = "You find an old wooden chest, covered in cobwebs. You open the lid carefully.\n";

    _treasure.emplace_back(std::make_unique<InteractiveE>(26, 8, Entity::Type::kChest, msg));
    _treasure.back()->AddItem(std::move(coins));
    _treasure.back()->AddItem(std::move(armor));
  }
  {
    std::unique_ptr<InventoryItem> mcguffin = std::make_unique<InventoryItem>();
    mcguffin->name = "McGuffin";
    mcguffin->number = 1;    
    mcguffin->isMcGuffin = true;
    std::string msg = "Tucked against the back of the wall is an ancient chest. Once it was locked, but the it's hinges have long since been corroded away by rust. Inside, you find an odd little thing made entirely out off gold.\n";
    _treasure.emplace_back(std::make_unique<InteractiveE>(12, 2, Entity::Type::kChest, msg));
    _treasure.back()->AddItem(std::move(mcguffin));
  }

  {
    std::unique_ptr<InventoryItem> sword = std::make_unique<InventoryItem>();
    sword->name = "Magic Blade";
    sword->number = 1;
    sword->attack_mod = 8;
    sword->isWeapon = true;  
    std::string msg = "As you approach, the apparition lifts its head. THe undead eyes seem to freeze your soul. As it speaks, it's voice is but the memory of a distance echo. 'Fellow traveler! I am the unlucky soul who ventured into this cave before you. Release me from this curse. Take my blade and avenge my death!' With a sigh of sadness and relief, the apparition vanishes into thin air.\n";
    
    _treasure.emplace_back(std::make_unique<InteractiveE>(46,34, Entity::Type::kLoot, msg));
    _treasure.back()->AddItem(std::move(sword));
  }
  
}