#ifndef GAME_TYPES_H
#define GAME_TYPES_H

enum GameState { MAIN_MENU, PLAYING, PAUSED, STAGE_COMPLETED };
enum MenuSubState { SUB_MAIN, SUB_LOAD_SELECT }; 
enum SaveSubState { SAVE_INACTIVE, SAVE_SLOT_SELECT, SAVE_OVERWRITE_CONFIRM, SAVE_DELETE_CONFIRM };
enum EngineMode { EDIT_MODE, PLAY_MODE };

enum AssetType { ASSET_BLOCK, ASSET_ROCK, ASSET_HILL, ASSET_TREE, ASSET_TRAP, ASSET_GOAL };

struct EnvironmentBlock {
    AssetType type;
    float x, y, z;
    float scaleX, scaleY, scaleZ;
    float r, g, b;
    bool climbable; 
};

struct EnemySnapshot {
    float spawnX, spawnZ;
};

struct Projectile {
    float x, y, z;
    float dirX, dirZ;
    float speed;
    float lifetime;
    bool isAlive;
};

enum EventType { EVENT_SPAWN_ENEMY, EVENT_SPAWN_ASSET, EVENT_CLEAR_BLOCKS, EVENT_PURGE_ENEMIES, EVENT_PLAYER_DAMAGE, EVENT_DESTROY_BLOCK };

struct EngineEvent {
    EventType type;
    AssetType assetType;
    float paramX = 0.0f;
    float paramY = 0.0f;
    float paramZ = 0.0f;
    int paramValue = 0;
    int targetBlockIndex = -1; 
};

#endif