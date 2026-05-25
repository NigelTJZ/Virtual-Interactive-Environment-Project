#ifndef BEHAVIOR_SYSTEM_H
#define BEHAVIOR_SYSTEM_H

#include "Engine/GameTypes.h"
#include "Engine/Core/ECS.h"
#include "Engine/Core/SpatialGrid.h"
#include <vector>

void UpdateBehaviorSystem(
    Registry& ecs, 
    std::vector<Entity>& enemyList, 
    const std::vector<EnvironmentBlock>& worldBlocks,
    Entity mario, 
    Entity luigi, 
    float deltaTime, 
    float globalEnemySpeed, 
    float luigiMovementSpeed,
    bool luigiIsHelping, 
    bool enableSpatialGrid, 
    SpatialGrid& spatialGrid, 
    bool enableEnemyJumping,
    bool isPlayerInvulnerable, 
    int& playerHealth, 
    float& damageCooldown, 
    std::vector<EngineEvent>& eventQueue, 
    EngineMode currentMode,
    bool& luigiHasGun, 
    float& luigiShootingCooldown, 
    std::vector<Projectile>& bulletList
);

#endif