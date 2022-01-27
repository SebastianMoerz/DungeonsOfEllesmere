#ifndef TILE_TYPES_H
#define TILE_TYPES_H

namespace MapTiles
{   
    enum class VicinityTileType { kInside, kFringe, kOutside, kEdgeNorth, kEdgeSouth, kEdgeEast, kEdgeWest, kEdgeNorthEast, kEdgeNorthWest, kEdgeSouthEast, kEdgeSouthWest} ; 
    enum class Type { kBedrock, kInnerWall, kOuterWall, kFloor, kGras };    
    
} // namespace MapTiles

#endif