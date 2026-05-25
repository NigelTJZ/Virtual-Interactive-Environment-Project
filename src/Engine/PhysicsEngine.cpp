#include "Engine/PhysicsEngine.h"
#include <cmath>

void Resolve3DCollision(float& posX, float& posY, float& posZ, float& velY, bool& grounded, const EnvironmentBlock& block, float extX, float extY, float extZ, int blockIdx, std::vector<EngineEvent>& eventQueue, EngineMode currentMode, int initiator) {
    if (block.type == ASSET_GOAL) return; 
    int iterations = (block.type == ASSET_HILL) ? 3 : 1;

    for (int i = 0; i < iterations; ++i) {
        float bScaleX = block.scaleX; float bScaleY = block.scaleY; float bScaleZ = block.scaleZ; float bX = block.x; float bY = block.y; float bZ = block.z;

        if (block.type == ASSET_HILL) {
            bY = block.y + (i * 0.4f); bScaleX = block.scaleX - (i * 1.0f); bScaleZ = block.scaleZ - (i * 1.0f); bScaleY = 0.4f;
        } else if (block.type == ASSET_TREE) { bScaleY = 2.5f; }

        float blockMinX = bX - (bScaleX * 0.5f); float blockMaxX = bX + (bScaleX * 0.5f);
        float blockMinZ = bZ - (bScaleZ * 0.5f); float blockMaxZ = bZ + (bScaleZ * 0.5f);
        float blockMaxY = (block.type == ASSET_HILL) ? bY + 0.2f : bY + (bScaleY * 0.5f);
        float blockMinY = bY - (bScaleY * 0.5f);

        float entMinX = posX - extX; float entMaxX = posX + extX; float entMinZ = posZ - extZ; float entMaxZ = posZ + extZ; float entMinY = posY - extY; float entMaxY = posY + extY;

        if (entMaxX > blockMinX && entMinX < blockMaxX && entMaxZ > blockMinZ && entMinZ < blockMaxZ && entMaxY > blockMinY && entMinY < blockMaxY) {
            float overlapX = (posX > bX) ? (blockMaxX - entMinX) : (entMaxX - blockMinX);
            float overlapZ = (posZ > bZ) ? (blockMaxZ - entMinZ) : (entMaxZ - blockMinZ);
            float overlapY = blockMaxY - entMinY;

            float bottomCeilingDiff = std::abs(entMaxY - blockMinY);
            if (block.type == ASSET_BLOCK && bottomCeilingDiff < 0.25f && velY > 0.0f && currentMode == PLAY_MODE) {
                velY = -2.0f; 
                eventQueue.push_back({ EVENT_DESTROY_BLOCK, ASSET_BLOCK, 0, 0, 0, initiator, blockIdx });
                return;
            }

            float stepDiff = blockMaxY - (posY - extY);
            if (block.climbable && stepDiff > 0.0f && stepDiff <= 0.45f) {
                posY = blockMaxY + extY; velY = 0.0f; grounded = true;
            } 
            else if (overlapY < overlapX && overlapY < overlapZ && velY <= 0.0f && block.climbable) {
                posY = blockMaxY + extY; velY = 0.0f; grounded = true;
            } else {
                if (overlapX < overlapZ) posX += (posX > bX) ? overlapX : -overlapX;
                else posZ += (posZ > bZ) ? overlapZ : -overlapZ;
            }
        }
    }
}