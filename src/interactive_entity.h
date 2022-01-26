#ifndef INTERACTIVE_ENTITY_H
#define INTERACTIVE_ENTITY_H

#include <vector>
#include <string>
#include <memory>
#include "SDL.h"
#include "entity.h"
#include "player.h"

#include <fstream>
#include <iostream>
using std::ifstream;

// an object on the game map which the player can interact with
class InteractiveE : public Entity {
  public:
    // POSSIBLE IMPROVEMENT: split into separate classes NPC and Treasure:
    // constructor for questgiver (NPC) - a stationary entity that provides dialogue lines upon collision with player
    InteractiveE(int x, int y, std::string filename); 
   
    // constructor for treasure - a stationary entity that moves a item into the player's inventory upon collision
    InteractiveE(int x, int y, Type type, std::string pickUpText);

    // after creating an (empty) instance, lootable objects need to be added     
    void AddItem(std::unique_ptr<InventoryItem> item) { _treasure.emplace_back(std::move(item)); }

    // interaction methods    
    void Interact(Player *player);    
    
    // setters & getters
    void SetAsMainQuestGiver() { _isMainQuestGiver = true; }  // marks an NPC as linked to the main quest
           
  private:
    // for type kTreasure, kLoot, kChest
    std::vector<std::unique_ptr<InventoryItem>> _treasure{};  // lootable objects are stored here
    std::string _pickUpText{""};                              // print this text if player picks up the treasure
    void PickUpItems(Player *player);                         // move content of _treasure to player's inventory    
        
    // for type kNPC
    bool _isMainQuestGiver{false};                                          // is NPC marked as linked to main quest?
    std::string _filename{""};                                              // location of dialogue definition 
    std::size_t _annoyance{0};                                              // how much the player annoys the questgiver
    std::vector<std::string> _questgiverDialogue;                           // main dialogue
    std::vector<std::string> _questgiverFinalResponse;                      // dialogue at game conclusion
    std::vector<std::string> ReadDialogueFromFile(std::string path);        // read dialogue lines which explain main quest
    std::vector<std::string> ReadFinalResponseFromFile(std::string path);   // read dialogue lines for game conclusion
    void Talk(Player *player);                                              // NPC talks to player
};

#endif