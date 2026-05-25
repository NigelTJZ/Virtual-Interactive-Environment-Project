#include "Engine/EventSystem.h"

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
) {
    for (const auto& ev : eventQueue) {
        switch (ev.type) {
            case EVENT_SPAWN_ENEMY:
                spawnEnemyCallback(ev.paramValue);
                break;
            case EVENT_SPAWN_ASSET:
                if (ev.assetType == ASSET_BLOCK) { worldBlocks.push_back({ ASSET_BLOCK, ev.paramX, ev.paramY, ev.paramZ, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, true }); } 
                else if (ev.assetType == ASSET_ROCK) { worldBlocks.push_back({ ASSET_ROCK, ev.paramX, ev.paramY, ev.paramZ, 2.2f, 1.2f, 2.2f, 0.0f, 0.0f, 0.0f, true }); } 
                else if (ev.assetType == ASSET_HILL) { worldBlocks.push_back({ ASSET_HILL, ev.paramX, ev.paramY, ev.paramZ, 5.0f, 0.5f, 5.0f, 0.0f, 0.0f, 0.0f, true }); } 
                else if (ev.assetType == ASSET_TREE) { worldBlocks.push_back({ ASSET_TREE, ev.paramX, ev.paramY, ev.paramZ, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, false }); } 
                else if (ev.assetType == ASSET_TRAP) { worldBlocks.push_back({ ASSET_TRAP, ev.paramX, ev.paramY, ev.paramZ, 2.5f, 0.1f, 2.5f, 0.0f, 0.0f, 0.0f, true }); } 
                else if (ev.assetType == ASSET_GOAL) { worldBlocks.push_back({ ASSET_GOAL, ev.paramX, ev.paramY, ev.paramZ, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, false }); }
                break;
            case EVENT_DESTROY_BLOCK:
                if (ev.targetBlockIndex >= 0 && ev.targetBlockIndex < worldBlocks.size()) {
                    if (worldBlocks[ev.targetBlockIndex].type == ASSET_BLOCK) {
                        if (ev.paramValue == 2) { luigiHasGun = true; } 
                        else { if (playerGunLevel >= 5) { weaponMessageTimer = 3.0f; playerGunLevel = 5; } else { playerGunLevel++; } }
                    }
                    worldBlocks.erase(worldBlocks.begin() + ev.targetBlockIndex);
                }
                break;
            case EVENT_CLEAR_BLOCKS: worldBlocks.clear(); break;
            case EVENT_PURGE_ENEMIES: for (Entity e : enemyList) { ecs.getEnemyAI(e).isAlive = false; } enemyList.clear(); break;
            case EVENT_PLAYER_DAMAGE: playerHealth -= 1; damageCooldown = 1.5f; if (playerHealth <= 0) playerHealth = 0; break;
            default: break;
        }
    }
}