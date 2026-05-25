#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include "Engine/GameTypes.h"
#include "Engine/Core/ECS.h"
#include <vector>
#include <string>

bool CheckFileExists(const std::string& fileName);
void executeSaveOperation(int slotNumber, float playerMovementSpeed, float luigiMovementSpeed, float globalEnemySpeed, int playerMaxHealth, bool isGunEnabled, int playerGunLevel, const std::vector<Entity>& enemyList, Registry& ecs, const std::vector<EnvironmentBlock>& worldBlocks);
void executeLoadOperation(int slotNumber, float& playerMovementSpeed, float& luigiMovementSpeed, float& globalEnemySpeed, int& playerMaxHealth, bool& isGunEnabled, int& playerGunLevel, std::vector<Entity>& enemyList, Registry& ecs, std::vector<EnvironmentBlock>& worldBlocks);

#endif