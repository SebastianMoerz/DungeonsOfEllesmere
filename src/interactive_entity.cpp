#include "interactive_entity.h"
#include <iostream>

// constructor for questgiver - a stationary entity that provides dialogue lines upon collision with player
InteractiveE::InteractiveE(int x, int y, std::string filename) : Entity(x, y, Type::kNPC), _filename(filename)
{
    SetBlocksPath(true);
    _questgiverDialogue = ReadDialogueFromFile(filename);
    _questgiverFinalResponse = ReadFinalResponseFromFile (filename);       
}

// constructor for treasure - a stationary entity that moves a item into the player's inventory upon collision
InteractiveE::InteractiveE(int x, int y, Type type, std::string pickUpText) : Entity(x, y, type), _pickUpText(pickUpText) { 
    // set block path property if not a chest
    if (type == Type::kLoot || type == Type::kTreasure) 
    SetBlocksPath(false);     
}

// main interface to player 
void InteractiveE::Interact(Player *player) 
{
    if (this->GetType() == Type::kNPC) { Talk(player); }
    if (this->GetType() == Type::kChest) { PickUpItems(player); }
    if (this->GetType() == Type::kTreasure) { 
        PickUpItems(player);
        MarkForErasure(); 
    }
    if (this->GetType() == Type::kLoot) { 
        PickUpItems(player);
        MarkForErasure(); 
    }    
}

// move content of _treasure to player's inventory    
void InteractiveE::PickUpItems(Player *player) { 
    std::cout << "---------------" << std::endl;
    std::cout << _pickUpText; //no endline here to prevent line breaks if msg is empty.

    if(_treasure.empty()) {
        std::cout << "There are no items to be picked up." << std::endl;
    } else {
        for (std::unique_ptr<InventoryItem> &item : _treasure) {
            player->receiveItem(std::move(item));
        }
    }
    _treasure.clear();
}

// talk to player
// ROOM FOR IMPROVEMENT: currently only supports only one type of dialogue (main quest) and has hard coded reward methods. generic interface needed if game was to be extended.
void InteractiveE::Talk(Player *player) 
{
    // confirm that player has not found the main quest object yet
    if (!player->hasMcGuffin()) {            
        std::cout << "---------------" << std::endl;
        // if (_annoyance >= _questgiverDialogue.size()) { return; }        // uncomment to avoid the intentional segmentation fault caused by the line below.
        std::cout << _questgiverDialogue.at(_annoyance++) << std::endl;		// this line WILL eventually cause an intentional segmentation fault. it's not a bug, it's a feature ;D	
        
        // hard coded reward for dialogue options. eventually this should be created from a file parser 
        if (_annoyance == 6) { player->ReceiveXP(20); }
        if (_annoyance == 7) {
            std::unique_ptr<InventoryItem> coins = std::make_unique<InventoryItem>();
            coins->name = "Gold Coin";
            coins->number = 25;
            player->receiveItem(std::move(coins));
            player->ReceiveXP(30);
        }
    }
    // if player returns to NPC after finding main quest object
    if (player->hasMcGuffin()) {
        
        if (_isMainQuestGiver) { player->SetQuestComplete(); }

        // init object for hard coded reward. eventually this should be created from a file parser 
        std::unique_ptr<InventoryItem> coins = std::make_unique<InventoryItem>();
        coins->name = "Gold Coin";

        std::cout << "---------------" << std::endl;
        // response & reward depend on how much the player has annoyed the quest giver
        switch (_annoyance) {
            case 0 ... 2: 
                std::cout << _questgiverFinalResponse.at(0) << std::endl;
                coins->number = 50;
                player->receiveItem(std::move(coins));
                player->ReceiveXP(250);
                break;

            case 3 ... 5:
                std::cout << _questgiverFinalResponse.at(1) << std::endl;
                coins->number = 30;
                player->receiveItem(std::move(coins));
                player->ReceiveXP(250);
                break;
                    
            case 6:
                std::cout << _questgiverFinalResponse.at(2) << std::endl;
                coins->number = 50;
                player->receiveItem(std::move(coins));
                player->ReceiveXP(250);
                break;
                
            case 7 ... 9:
                std::cout << _questgiverFinalResponse.at(3) << std::endl;
                coins->number = 25;
                player->receiveItem(std::move(coins));
                player->ReceiveXP(250);
                break;

            case 10 ... 12:
                std::cout << _questgiverFinalResponse.at(4) << std::endl;
                coins->number = 25;
                player->receiveItem(std::move(coins));
                player->ReceiveXP(250);
                break;
                    
            default:
                std::cout << _questgiverFinalResponse.at(5) << std::endl;
                coins->number = 25;
                player->receiveItem(std::move(coins));
                player->ReceiveXP(250);
                break;
        }
    }
}	

// read dialogue lines which explain main quest
std::vector<std::string> InteractiveE::ReadDialogueFromFile(std::string path) {
    ifstream textfile (path);
    std::vector<std::string> dialogue{};
    if (textfile) {
        std::string line;
        while (getline(textfile, line)) {
            // break if end of dialogue paragraph was reached
            if (line == "---") {break;}             
            dialogue.push_back(line);      
        }
    }
    else {
        std::cout << "Error: File " << path << " could not be opened!" << std::endl;
    }
    return dialogue;
}

// read dialogue lines for game conlcusion
std::vector<std::string> InteractiveE::ReadFinalResponseFromFile(std::string path) {
    ifstream textfile (path);
    std::vector<std::string> response{};
    if (textfile) {
        std::string line;
        while (getline(textfile, line)) {
            // break if end of dialogue paragraph was reached - response paragraph will now begin
            if (line == "---") {break;}
        }
        while (getline(textfile, line)) {    
            response.push_back(line);      
        }
    }
    return response;
}    