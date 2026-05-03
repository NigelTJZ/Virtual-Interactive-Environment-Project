#include "Engine/Core/SpatialGrid.h"
#include <cmath>

SpatialGrid::SpatialGrid(float cellSize) : m_cellSize(cellSize) {}

void SpatialGrid::clear() {
    m_cells.clear();
}

uint64_t SpatialGrid::getCellKey(float x, float z) {
    // Convert float coordinates into integer grid indices
    int cx = static_cast<int>(std::floor(x / m_cellSize));
    int cz = static_cast<int>(std::floor(z / m_cellSize));
    
    // Bitwise shift to combine two 32-bit ints into one 64-bit int for fast hashing
    return (static_cast<uint64_t>(static_cast<uint32_t>(cx)) << 32) | static_cast<uint32_t>(cz);
}

void SpatialGrid::insert(Entity entity, float x, float z) {
    m_cells[getCellKey(x, z)].push_back(entity);
}

std::vector<Entity> SpatialGrid::getNearbyEntities(float x, float z, float radius) {
    std::vector<Entity> nearby;
    int searchCells = static_cast<int>(std::ceil(radius / m_cellSize));
    
    int cx = static_cast<int>(std::floor(x / m_cellSize));
    int cz = static_cast<int>(std::floor(z / m_cellSize));

    // Check the current cell and surrounding cells
    for (int dx = -searchCells; dx <= searchCells; ++dx) {
        for (int dz = -searchCells; dz <= searchCells; ++dz) {
            uint64_t key = (static_cast<uint64_t>(static_cast<uint32_t>(cx + dx)) << 32) | static_cast<uint32_t>(cz + dz);
            auto it = m_cells.find(key);
            if (it != m_cells.end()) {
                nearby.insert(nearby.end(), it->second.begin(), it->second.end());
            }
        }
    }
    return nearby;
}