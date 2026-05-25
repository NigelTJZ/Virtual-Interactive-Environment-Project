#include "Engine/FileHandler.h"
#include <fstream>

bool CheckFileExists(const std::string& fileName) {
    std::ifstream f(fileName.c_str());
    return f.good();
}

void executeSaveOperation(int slotNumber, float playerMovementSpeed, float luigiMovementSpeed, float globalEnemySpeed, int playerMaxHealth, bool isGunEnabled, int playerGunLevel, const std::vector<Entity>& enemyList, Registry& ecs, const std::vector<EnvironmentBlock>& worldBlocks) {
    std::string pathName = "slot" + std::to_string(slotNumber) + ".env";
    std::ofstream out(pathName);
    if (!out.is_open()) return;

    out << playerMovementSpeed << " " << luigiMovementSpeed << " " << globalEnemySpeed << " "
        << playerMaxHealth << " " << isGunEnabled << " " << playerGunLevel << "\n";

    int liveEnemiesCount = 0;
    for (Entity e : enemyList) { if (ecs.getEnemyAI(e).isAlive) liveEnemiesCount++; }
    out << liveEnemiesCount << "\n";
    for (Entity e : enemyList) {
        if (ecs.getEnemyAI(e).isAlive) {
            out << ecs.getTransform(e).x << " " << ecs.getTransform(e).z << "\n";
        }
    }

    out << worldBlocks.size() << "\n";
    for (const auto& block : worldBlocks) {
        out << static_cast<int>(block.type) << " " << block.x << " " << block.y << " " << block.z << " "
            << block.scaleX << " " << block.scaleY << " " << block.scaleZ << " "
            << block.r << " " << block.g << " " << block.b << " " << block.climbable << "\n";
    }
    out.close();
}

void executeLoadOperation(int slotNumber, float& playerMovementSpeed, float& luigiMovementSpeed, float& globalEnemySpeed, int& playerMaxHealth, bool& isGunEnabled, int& playerGunLevel, std::vector<Entity>& enemyList, Registry& ecs, std::vector<EnvironmentBlock>& worldBlocks) {
    std::string pathName = "slot" + std::to_string(slotNumber) + ".env";
    std::ifstream in(pathName);
    if (!in.is_open()) return;

    in >> playerMovementSpeed >> luigiMovementSpeed >> globalEnemySpeed >> playerMaxHealth >> isGunEnabled >> playerGunLevel;

    int enemyLoadCount = 0; in >> enemyLoadCount;
    for (int i = 0; i < enemyLoadCount; ++i) {
        float ex = 0, ez = 0; in >> ex >> ez;
        Entity e = ecs.createEntity(); Transform& t = ecs.getTransform(e); EnemyAI& ai = ecs.getEnemyAI(e);
        t.x = ex; t.y = 0.5f; t.z = ez; ai.spawnX = ex; ai.spawnZ = ez; ai.isAlive = true;
        enemyList.push_back(e);
    }

    size_t blockLoadCount = 0; in >> blockLoadCount;
    for (size_t i = 0; i < blockLoadCount; ++i) {
        int rawType = 0; EnvironmentBlock block;
        in >> rawType >> block.x >> block.y >> block.z >> block.scaleX >> block.scaleY >> block.scaleZ >> block.r >> block.g >> block.b >> block.climbable;
        block.type = static_cast<AssetType>(rawType);
        worldBlocks.push_back(block);
    }
    in.close();
}