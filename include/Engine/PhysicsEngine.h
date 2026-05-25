#ifndef PHYSICS_ENGINE_H
#define PHYSICS_ENGINE_H

#include "Engine/GameTypes.h"
#include <vector>

void Resolve3DCollision(float& posX, float& posY, float& posZ, float& velY, bool& grounded, const EnvironmentBlock& block, float extX, float extY, float extZ, int blockIdx, std::vector<EngineEvent>& eventQueue, EngineMode currentMode, int initiator);

#endif