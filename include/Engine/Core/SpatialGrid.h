#pragma once
#include "Engine/Core/ECS.h"
#include <vector>
#include <unordered_map>
#include <cstdint>

class SpatialGrid {
public:
    // cellSize determines how large each grid square is
    SpatialGrid(float cellSize);
    
    // Clears the grid every frame
    void clear();
    
    // Inserts an entity into the correct grid cell based on its x/z position
    void insert(Entity entity, float x, float z);
    
    // Returns ONLY the entities inside the neighboring grid cells
    std::vector<Entity> getNearbyEntities(float x, float z, float radius);

private:
    float m_cellSize;
    
    // We use a 64-bit integer as a highly efficient hash key for 2D (X, Z) coordinates
    std::unordered_map<uint64_t, std::vector<Entity>> m_cells;
    
    uint64_t getCellKey(float x, float z);
};