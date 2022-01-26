#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "player.h"

class Controller {
 public:
  void HandleInput(bool &running, bool &paused, Player &player) const;

 private:  
};

#endif