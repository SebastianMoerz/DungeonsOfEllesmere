# DUNGEONS OF ELLESMERE - QUEST FOR THE GOLDEN McGUFFIN

## Project Description: Files, Class Structure and Expected Output

This game was created as part of the Capstone project of the [Udacity C++ Nanodegree Program](https://www.udacity.com/course/c-plus-plus-nanodegree--nd213) and is based on the "Snake" starter repo example (see: https://github.com/udacity/CppND-Capstone-Snake-Game)


### How to Play
"Dungeons of Ellesmere" is a proof-of-concept RPG-style minigame written in C++ using the SDL-Library.
The player takes on the role of a travelling adventurer on a (rather generic) quest to save the world - or at least collect the occasional loot that comes with defeating your enemies.

<img src="dungeonsOE.jpg"/>

The game uses a simple grid-based layout were each object is rendered as a colored square. The player's position is shown by a dark-blue square. Use the arrow keys to move the icon on the game map.
You can interact with objects on the game map by moving the player to their position (i.e. try to move on top of them - just being next an object doesn' trigger an interaction).
Depending on the type of object, the player can do the following things:
  talk to friendly NPCs (light blue squares)
  collect treasure (yellow squares)
  open treasure chests (brown squares)
  fight opponents (red squares)
Note that in order to fight an opponent, you have to repeatedly move toward it using the arrow keys. Each key press equals one strike.

Status information and dialogue is printed on the console, so make sure to keep an eye on the text output as well as the game map!

<img src="console.jpg"/>

Game control:
At any time during the game, you can:
  Press (p) for pausing / unpausing the game
  Press (i) to take a look at your inventory
  Press (c) to check your adventurer's health
  Press (1-9) to use or equip items from your inventory

Once the game ends, you have to restart the application to continue.

Now have fun and save the world!


### File & Class Structure
Base class for objects on the game map:
	entity.cpp
	entity.h

Class definition for objects that can interact with the player
	interactive_entity.cpp
	interactive_entity.h

Base class for objects that can engage in combat:
	combattant.cpp
	combattant.h
	
Class definition of hostile objects that will attack the player
	opponent.cpp
	opponent.h
	
Class definition of the player object
	player.cpp
	player.hostile
	
Helper functions for directed movement (based on A-Star search algorithm)
	game_utils.h
	
Class definition for the game object (controls game loop)
	game.cpp
	game.h 
	
Class definition for controller (handles user input)
	controller.cpp
	controller.h 
	
Class definition of renderer
	renderer.cpp
	renderer.h 
	
Main Programm
	main.cpp
	
Text file containing Questgiver dialogue:
	dialogue.txt

## Satisfaction of Rubric Points
see project rubric: (https://review.udacity.com/#!/rubrics/2533/view)
### Loops, Functions, I/O:
REQ: "A variety of control structures are used in the project."
some examples: 
	if - else if - else (game.cpp, 262ff)
	while(true) - break (game.cpp, 226ff)
	switch - case (interactive_entity.cpp, 80ff)
	for-loop (game.cpp, 342ff)

REQ: "The project code is clearly organized into functions."
  see: function declaration in header-files

REQ: "The project reads data from an external file or writes data to a file as part of the necessary operation of the program."
	see: interactive_entity.cpp, lines 127ff

REQ: "The project accepts input from a user as part of the necessary operation of the program."
	see file: controller.cpp

### Object Oriented Programming
REQ: "The project code is organized into classes with class attributes to hold the data, and class methods to perform tasks."
	see the different file pairs (*h. and *.cpp), e.g. player.h and player.cpp


REQ: "All class data members are explicitly specified as public, protected, or private."
	see header-files

REQ: "All class members that are set to argument values are initialized through member initialization lists."
	see entity.h, game.cpp, interactive_entity.cpp, opponent.h, player.h (renderer.cpp was taken over from starter repo without modifications)

REQ: "All class member functions document their effects, either through function names, comments, or formal documentation. Member functions do not change program state in undocumented ways."
	comments were added to the code were necessary

REQ: "Appropriate data and functions are grouped into classes. Member data that is subject to an invariant is hidden from the user. State is accessed via member functions."
	examples: class Entity describes objects on the game map, class Combattant encapsulates data and methods for fight-handling

REQ: "Inheritance hierarchies are logical. Composition is used instead of inheritance when appropriate. Abstract classes are composed of pure virtual functions. Override functions are specified."
	interactive entities inherit from entities
	both player and opponent inherit from entities and combattant

REQ: "One function is overloaded with different signatures for the same function name."
	see e.g. DetectCollision() in game.h, lines 31ff)

REQ: "One member function in an inherited class overrides a virtual base class member function."
	see combattant.h (lines 31f), opponent.h (lines 32f), player.h (lines 41f)

### Memory Management
REQ: "At least two variables are defined as references, or two functions use pass-by-reference in the project code."
	function Renderer::Render takes references to vectors
	DetectCollision in game.cpp take references to vectors
	
REQ: "The project uses at least one smart pointer: unique_ptr, shared_ptr, or weak_ptr. The project does not use raw pointers."
	most in-game objects are stored in unique pointers.
	see e.g. moving of loot to player's inventory, (player.cpp, lines 50ff, especially line 70)

## Dependencies for Running Locally
* cmake >= 3.7
  * All OSes: [click here for installation instructions](https://cmake.org/install/)
* make >= 4.1 (Linux, Mac), 3.81 (Windows)
  * Linux: make is installed by default on most Linux distros
  * Mac: [install Xcode command line tools to get make](https://developer.apple.com/xcode/features/)
  * Windows: [Click here for installation instructions](http://gnuwin32.sourceforge.net/packages/make.htm)
* SDL2 >= 2.0
  * All installation instructions can be found [here](https://wiki.libsdl.org/Installation)
  >Note that for Linux, an `apt` or `apt-get` installation is preferred to building from source. 
* gcc/g++ >= 5.4
  * Linux: gcc / g++ is installed by default on most Linux distros
  * Mac: same deal as make - [install Xcode command line tools](https://developer.apple.com/xcode/features/)
  * Windows: recommend using [MinGW](http://www.mingw.org/)

## Basic Build Instructions

1. Clone this repo.
2. Make a build directory in the top level directory: `mkdir build && cd build`
3. Compile: `cmake .. && make`
4. Run it: `./Ellesmere`.


## CC Attribution-ShareAlike 4.0 International


Shield: [![CC BY-SA 4.0][cc-by-sa-shield]][cc-by-sa]

This work is licensed under a
[Creative Commons Attribution-ShareAlike 4.0 International License][cc-by-sa].

[![CC BY-SA 4.0][cc-by-sa-image]][cc-by-sa]

[cc-by-sa]: http://creativecommons.org/licenses/by-sa/4.0/
[cc-by-sa-image]: https://licensebuttons.net/l/by-sa/4.0/88x31.png
[cc-by-sa-shield]: https://img.shields.io/badge/License-CC%20BY--SA%204.0-lightgrey.svg
