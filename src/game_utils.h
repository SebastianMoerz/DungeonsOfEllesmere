#ifndef GAME_UTILS_H
#define GAME_UTILS_H

// helper functions for the game. currently includes an A-Star search algorithm to help moving the opponent toward the player
// the A-Star search code is a slightly modified version of the Udacity C++ Nanodegree course material 

// note: still somewhat buggy - NPCs tend to be blocked if the player hides behind an obstacle

#include <algorithm>  // for sort
#include <string>
#include <vector>
#include <memory>

#include "SDL.h"
#include "entity.h"
#include "tiletypes.h"

#include <fstream>
#include <iostream>
#include <sstream>

using std::sort;
using std::string;
using std::vector;
using std::abs;
using std::ifstream;
using std::istringstream;


namespace GameUtils {


    // -----------------------------
    // MAP MANAGEMENT & RENDER-UTILS
    // -----------------------------  

 

    // helper function: straightforward position-delta calculation
    SDL_Point GetVector (SDL_Point from, SDL_Point to) { return {to.x - from.x, to.y - from.y}; }

    // to be called after map creation for setting up the main resource for collision detection
    std::vector<std::vector<Entity::Type>> InitObstacleMap (std::vector<std::vector<MapTiles::Type>> &rendermap) {
        // init obstacle map
        std::vector<std::vector<Entity::Type>> obstaclemap{};

        for (std::vector<MapTiles::Type> row : rendermap) {
            std::vector<Entity::Type> targetrow{};
            for (MapTiles::Type tile : row) {
                if (tile == MapTiles::Type::kFloor) { targetrow.emplace_back(Entity::Type::kNone); }
                else { targetrow.emplace_back(Entity::Type::kObstacle); }
            }
            std::reverse(targetrow.begin(), targetrow.end());
            obstaclemap.push_back(targetrow);
        }
        std::reverse(obstaclemap.begin(), obstaclemap.end());
        return obstaclemap;
    }

    // get the vector of wall bricks from game map
    std::vector<std::unique_ptr<Entity>> GetWallFromMap(std::vector<std::vector<MapTiles::Type>> &map) {
        std::vector<std::unique_ptr<Entity>> wall = {};
        for (int x = 0; x < map.size(); x++) {
            for (int y = 0; y < (map[x]).size(); y++) {
                if (map[x][y] == MapTiles::Type::kOuterWall) { wall.emplace_back(std::make_unique<Entity>(x, y, Entity::Type::kObstacle)); }
            }
        }
        return wall;
    }


    // helper function for map parser
    // note: wall chars "#", "8", "-" could be eventually read from config-file
    std::vector<MapTiles::Type> ParseMapLine(std::string mapfileline) {
        istringstream linestream(mapfileline);
        char n;
        char c;
        std::vector<MapTiles::Type> row{};
        while (linestream >> n >> c && c == ',') { 
            std::cout << n;           
        if ( n == '#' ) { row.push_back(MapTiles::Type::kOuterWall); }
        else if ( n == '8' ) { row.push_back(MapTiles::Type::kInnerWall); }
        else if ( n == '-' ) { row.push_back(MapTiles::Type::kBedrock); }
        else if ( n == '+' ) { row.push_back(MapTiles::Type::kGras); }
        else { row.push_back(MapTiles::Type::kFloor); }
        }        
        std::cout << std::endl;
        return row;
    }

    // read game map from file
    std::vector<std::vector<MapTiles::Type>> GetRenderBaseMap(std::string filepath) {
        ifstream mapfile (filepath);
        std::vector<std::vector<MapTiles::Type>> tmp_map = {};    
        if (mapfile) {
            std::string mapfileline;
            while (getline(mapfile, mapfileline)) {
                tmp_map.push_back(ParseMapLine(mapfileline));
            }
            
            // init transposed map
            std::vector<std::vector<MapTiles::Type>> map(tmp_map[0].size(), std::vector<MapTiles::Type>());
            for (int x = 0; x < tmp_map.size(); x++) {
                for (int y = 0; y < tmp_map[x].size(); y++) {
                    map[y].push_back(tmp_map[x][y]);
                }
            }
            std::cout << map.size() << "/" << map[0].size() << std::endl;
            return map;                       
        }
        else {
            std::cout << "Error: Map file '" << filepath << "' could not be opened!" << std::endl;
        }       
        
        return tmp_map;
    }


    // check if an entity which is removed from player by "vectorToPlayer" is inside the vicinity map & shall be rendered
    bool isInPlayerVicinity (SDL_Point vectorToPlayer, std::vector<std::vector<MapTiles::VicinityTileType>> &map) {
        // better make center point calculation dynamic - this one is for 19x19 vicinity map
        SDL_Point centerpoint = {10,10};
        SDL_Point positionOnVicinityMap = {vectorToPlayer.x + centerpoint.x, vectorToPlayer.y + centerpoint.y};
        // first, check if coordinates are within bounds
        if (positionOnVicinityMap.x < 1 || positionOnVicinityMap.y < 1 || positionOnVicinityMap.x > map.size() || positionOnVicinityMap.y > map.size() ) {return false; }
        // if yes, check if type is "kInside" or "kFringe", i.e. object shall be rendered. note that origin offset (1,1) -> (0,0) needs to be accounted for
        if ( map[positionOnVicinityMap.x - 1][positionOnVicinityMap.y - 1] == MapTiles::VicinityTileType::kInside) { return true; }
        if ( map[positionOnVicinityMap.x - 1][positionOnVicinityMap.y - 1] == MapTiles::VicinityTileType::kFringe) { return true; }
        return false;
    }

    /*
    // return the points on the vicinity map which shall be explored if the player moves in a given direction
    std::vector<SDL_Point> SetExplorationArea(Entity::Direction direction, std::vector<std::vector<MapTiles::VicinityTileType>>& vicinitymap) {
        std::vector<SDL_Point> area;

        for (int x; x < vicinitymap.size(); x++) {
            for (int y; y < vicinitymap[x].size(); y++) {
                // if tile was marked to be explored upon moving in "direction", add coordinates to exploration area 
                if (direction == Entity::Direction::kUp && 
                (vicinitymap[x][y] == MapTiles::VicinityTileType::kEdgeNorth || vicinitymap[x][y] == MapTiles::VicinityTileType::kEdgeNorthEast|| vicinitymap[x][y] == MapTiles::VicinityTileType::kEdgeNorthWest)) {
                    area.push_back({x,y});
                }
                if (direction == Entity::Direction::kDown && 
                (vicinitymap[x][y] == MapTiles::VicinityTileType::kEdgeSouth || vicinitymap[x][y] == MapTiles::VicinityTileType::kEdgeSouthEast|| vicinitymap[x][y] == MapTiles::VicinityTileType::kEdgeSouthWest)) {
                    area.push_back({x,y});
                }
                if (direction == Entity::Direction::kLeft && 
                (vicinitymap[x][y] == MapTiles::VicinityTileType::kEdgeWest || vicinitymap[x][y] == MapTiles::VicinityTileType::kEdgeSouthWest|| vicinitymap[x][y] == MapTiles::VicinityTileType::kEdgeNorthWest)) {
                    area.push_back({x,y});
                }
                if (direction == Entity::Direction::kRight && 
                (vicinitymap[x][y] == MapTiles::VicinityTileType::kEdgeEast || vicinitymap[x][y] == MapTiles::VicinityTileType::kEdgeNorthEast|| vicinitymap[x][y] == MapTiles::VicinityTileType::kEdgeSouthEast)) {
                    area.push_back({x,y});
                }
            }
        }
        return area;
    }
    */

    // get the map of the players vicinity
    std::vector<std::vector<MapTiles::VicinityTileType>> GetVicinityMap() {

        // hardcoded definition of the vicinity map    
        std::vector<std::vector<int>> int_map =
                {{2,2,2,2,2,2,2,3,3,3,3,3,2,2,2,2,2,2,2},
                {2,2,2,2,2,2,8,1,1,1,1,1,7,2,2,2,2,2,2},
                {2,2,2,2,3,8,1,0,0,0,0,0,1,7,3,2,2,2,2},
                {2,2,2,8,1,1,0,0,0,0,0,0,0,1,1,7,2,2,2},
                {2,2,8,1,0,0,0,0,0,0,0,0,0,0,0,1,7,2,2},
                {2,6,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,5,2},
                {2,8,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,7,2},
                {6,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,5},
                {6,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,5},
                {6,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,5},
                {6,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,5},
                {6,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,5},
                {2,10,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,9,2},
                {2,6,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,5,2},
                {2,2,10,1,0,0,0,0,0,0,0,0,0,0,0,1,9,2,2},
                {2,2,2,10,1,1,0,0,0,0,0,0,0,1,1,9,2,2,2},
                {2,2,2,2,4,10,1,0,0,0,0,0,1,9,4,2,2,2,2},
                {2,2,2,2,2,2,10,1,1,1,1,1,9,2,2,2,2,2,2},
                {2,2,2,2,2,2,2,4,4,4,4,4,2,2,2,2,2,2,2}};

        // transpose matrix 
        // TODO: reverse does not transpose matrix: needs to be changed - but can also be skipped if exploration area is not used
        for (std::vector<int> row : int_map) { std::reverse(row.begin(), row.end()); }
        std::reverse(int_map.begin(), int_map.end());

        // init vicinity map
        std::vector<std::vector<MapTiles::VicinityTileType>> map{};
        for (int i = 0; i < 19; i++) {
            std::vector<MapTiles::VicinityTileType> row;
            for (int j = 0; j < 19; j++) {
                row.emplace_back(MapTiles::VicinityTileType::kInside);
            }
            map.push_back(row);
        }

        // assign map tile values
        for (int x = 0; x < 19; x++) {        
            for (int y = 0; y < 19; y++) {
                if ( int_map[x][y] == 1 ) { map[x][y] = MapTiles::VicinityTileType::kFringe; }
                else if ( int_map[x][y] == 2 ) { map[x][y] = MapTiles::VicinityTileType::kOutside; }
                else if ( int_map[x][y] == 3 ) { map[x][y] = MapTiles::VicinityTileType::kEdgeNorth; }
                else if ( int_map[x][y] == 4 ) { map[x][y] = MapTiles::VicinityTileType::kEdgeSouth; }
                else if ( int_map[x][y] == 5 )  { map[x][y] = MapTiles::VicinityTileType::kEdgeEast; }
                else if ( int_map[x][y] == 6 ) { map[x][y] = MapTiles::VicinityTileType::kEdgeWest; }
                else if ( int_map[x][y] == 7) { map[x][y] = MapTiles::VicinityTileType::kEdgeNorthEast; }
                else if ( int_map[x][y] == 8) { map[x][y] = MapTiles::VicinityTileType::kEdgeNorthWest; }
                else if ( int_map[x][y] == 9) { map[x][y] = MapTiles::VicinityTileType::kEdgeSouthEast; }
                else if ( int_map[x][y] == 10) { map[x][y] = MapTiles::VicinityTileType::kEdgeSouthWest; }
            }        
        }
        return map;
    }


    // -----------------------
    // A-STAR SEARCH ALGORITHM
    // -----------------------
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