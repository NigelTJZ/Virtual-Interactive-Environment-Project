#ifndef EVENT_SYSTEM_H
#define EVENT_SYSTEM_H

#include "Engine/GameTypes.h"
#include "Engine/Core/ECS.h"
#include <vector>
#include <functional> // Required for C++17 generic lambda callbacks

void ProcessEventSystem(
    std::vector<EngineEvent>& eventQueue,
    std::vector<EnvironmentBlock>& worldBlocks,
    std::vector<Entity>& enemyList,
    Registry& ecs,
    int playerMaxHealth,
    int& playerHealth,
    float& damageCooldown,
    int& playerGunLevel,
    float& weaponMessageTimer,
    bool& luigiHasGun,
    std::function<void(int)> spawnEnemyCallback // Fixed: explicit signature type wrapper
);

#endif