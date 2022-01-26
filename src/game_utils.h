#ifndef GAME_UTILS_H
#define GAME_UTILS_H

// helper functions for the game. currently includes an A-Star search algorithm to help moving the opponent toward the player
// the A-Star search code is a slightly modified version of the Udacity C++ Nanodegree course material 

// note: still somewhat buggy - NPCs tend to be blocked if the player hides behind an obstacle

#include <algorithm>  // for sort
#include <string>
#include <vector>
#include "SDL.h"
#include "entity.h"

using std::sort;
using std::string;
using std::vector;
using std::abs;

namespace GameUtils {

    // directional deltas
    const int delta[4][2]{{-1, 0}, {0, -1}, {1, 0}, {0, 1}};

    // Compare the F values of two cells.
    bool Compare(const vector<int> a, const vector<int> b) {
        int f1 = a[2] + a[3]; // f1 = g1 + h1
        int f2 = b[2] + b[3]; // f2 = g2 + h2
        return f1 > f2; 
    }

    // Sort the two-dimensional vector of ints in descending order.
    void CellSort(vector<vector<int>> *v) { sort(v->begin(), v->end(), Compare); }

    // Calculate the manhattan distance
    int Heuristic(int x1, int y1, int x2, int y2) { return abs(x2 - x1) + abs(y2 - y1); }

    // nCheck that a cell is valid: on the grid, not an obstacle, and clear. 
    bool CheckValidCell(int x, int y, vector<vector<Entity::Type>> &grid) {
        bool on_grid_x = (x >= 0 && x < grid.size());
        bool on_grid_y = (y >= 0 && y < grid[0].size());
        if (on_grid_x && on_grid_y)
            //note that grid coordinates are of format (y,x), not (x,y)
            return grid[y][x] != Entity::Type::kObstacle;
        return false;
    }

    // Add a node to the open list and mark it as open. 
    void AddToOpen(int x, int y, int g, int h, vector<vector<int>> &openlist, vector<vector<Entity::Type>> &grid) {
        // Add node to open vector, and mark grid cell as closed.        
        openlist.push_back(vector<int>{x, y, g, h});
        // treat already visited cells as obstacle, i.e. don't visit them again (not very precise, but compatible with definition of entity type enum)
        //note that grid coordinates are of format (y,x), not (x,y)
        grid[y][x] = Entity::Type::kObstacle; 
    }

    // Expand current nodes's neighbors and add them to the open list.
    void ExpandNeighbors(const vector<int> &current, SDL_Point target, vector<vector<int>> &openlist, vector<vector<Entity::Type>> &grid) {
        // Get current node's data.
        int x = current[0];
        int y = current[1];
        int g = current[2];

        // Loop through current node's potential neighbors.
        for (int i = 0; i < 4; i++) {
            int x2 = x + delta[i][0];
            int y2 = y + delta[i][1];

            // Check that the potential neighbor's x2 and y2 values are on the grid and not closed.
            if (CheckValidCell(x2, y2, grid)) {
                // Increment g value and add neighbor to open list.
                int g2 = g + 1;
                int h2 = Heuristic(x2, y2, target.x, target.y);
                AddToOpen(x2, y2, g2, h2, openlist, grid);
            }
        }
    }

    // Implementation of A* search algorithm. Returns next step toward target as SDL_Point
    // To be called from game.cpp
    SDL_Point MoveTowardTarget(vector<vector<Entity::Type>> grid, SDL_Point init, SDL_Point target, int maxDist) {
        // Create the vector of open nodes.
        vector<vector<int>> open {};
        vector<SDL_Point> path {};        
        
        // Initialize the starting node.
        int x = init.x;
        int y = init.y;
        int g = 0;
        int h = Heuristic(x, y, target.x, target.y);
        // abort calculation if distance to target is to high (otherwise fps will drop significantly, especially for multiple opponents)
        if (h > maxDist) { return init;}
        AddToOpen(x, y, g, h, open, grid);

        while (open.size() > 0) {
            // Get the next node
            CellSort(&open);
            auto current = open.back();
            open.pop_back();
            x = current[0];
            y = current[1];
            path.push_back({x,y});            
            
            // Check if we're done. If yes, return first step toward target
            if (x == target.x && y == target.y && !path.empty()) { return path.at(1); }   
            // If we're not done, expand search to current node's neighbors.
            ExpandNeighbors(current, target, open, grid);
        }
        // We've run out of new nodes to explore and haven't found a path.
        // in this case, return unchanged starting position
        return init;
    }

} // end namespace GameUtils

#endif