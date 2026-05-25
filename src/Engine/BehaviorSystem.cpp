#include "Engine/BehaviorSystem.h"
#include "Engine/PhysicsEngine.h"
#include <cmath>

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
) {
    Transform& marioTransform = ecs.getTransform(mario);
    PhysicsBody& marioPhysics = ecs.getPhysicsBody(mario); // Fetched to manage jump bounce mechanics
    Transform& luigiTransform = ecs.getTransform(luigi);
    PhysicsBody& luigiPhysics = ecs.getPhysicsBody(luigi);

    if (currentMode == PLAY_MODE) {
        float targetLocationX = marioTransform.x;
        float targetLocationZ = marioTransform.z;
        bool activeGoalConfirmed = false;
        Entity selectedEnemyTarget = -1;
        float minimumTrackingRange = 9999.0f;

        if (luigiIsHelping) {
            for (Entity e : enemyList) {
                if (ecs.getEnemyAI(e).isAlive) {
                    Transform& et = ecs.getTransform(e);
                    float deltaX = et.x - luigiTransform.x; float deltaZ = et.z - luigiTransform.z;
                    float computedDistance = std::sqrt(deltaX * deltaX + deltaZ * deltaZ);
                    if (computedDistance < minimumTrackingRange) {
                        minimumTrackingRange = computedDistance;
                        targetLocationX = et.x; targetLocationZ = et.z;
                        selectedEnemyTarget = e; activeGoalConfirmed = true;
                    }
                }
            }

            if (!luigiHasGun) {
                for (const auto& itemBox : worldBlocks) {
                    if (itemBox.type == ASSET_BLOCK) {
                        float deltaX = itemBox.x - luigiTransform.x; float deltaZ = itemBox.z - luigiTransform.z;
                        float computedDistance = std::sqrt(deltaX * deltaX + deltaZ * deltaZ);
                        if (computedDistance < minimumTrackingRange) {
                            minimumTrackingRange = computedDistance;
                            targetLocationX = itemBox.x; targetLocationZ = itemBox.z;
                            selectedEnemyTarget = -1; activeGoalConfirmed = true;
                        }
                    }
                }
            }
        }

        if (activeGoalConfirmed && luigiIsHelping) {
            float deltaX = targetLocationX - luigiTransform.x; float deltaZ = targetLocationZ - luigiTransform.z;
            if (minimumTrackingRange > 0.1f) {
                float speedModifier = luigiPhysics.isGrounded ? 1.0f : 0.35f;
                luigiPhysics.velocityX = (deltaX / minimumTrackingRange) * luigiMovementSpeed * speedModifier;
                luigiPhysics.velocityZ = (deltaZ / minimumTrackingRange) * luigiMovementSpeed * speedModifier;

                if (minimumTrackingRange > 0.75f) {
                    luigiTransform.rotationY = atan2(deltaX, deltaZ) * (180.0f / 3.14159265f);
                }

                if (selectedEnemyTarget == -1 && minimumTrackingRange < 1.5f) {
                    float slowdownFactor = minimumTrackingRange / 1.5f;
                    luigiPhysics.velocityX *= slowdownFactor; luigiPhysics.velocityZ *= slowdownFactor;
                }
            } else {
                luigiPhysics.velocityX = 0.0f; luigiPhysics.velocityZ = 0.0f;
            }

            if (selectedEnemyTarget != -1) {
                if (luigiHasGun) {
                    if (luigiShootingCooldown > 0.0f) luigiShootingCooldown -= deltaTime;
                    else if (minimumTrackingRange < 22.0f) {
                        Projectile bullet;
                        bullet.x = luigiTransform.x; bullet.y = luigiTransform.y + 0.1f; bullet.z = luigiTransform.z;
                        bullet.speed = 35.0f; bullet.lifetime = 1.5f; bullet.isAlive = true;
                        bullet.dirX = deltaX / minimumTrackingRange; bullet.dirZ = deltaZ / minimumTrackingRange;
                        bulletList.push_back(bullet);
                        luigiShootingCooldown = 0.38f;
                    }
                } else {
                    if (minimumTrackingRange < 1.8f && luigiPhysics.isGrounded) {
                        luigiPhysics.velocityY = 11.0f; luigiPhysics.isGrounded = false;
                    }
                }
            } else {
                if (minimumTrackingRange < 0.25f && luigiPhysics.isGrounded) {
                    luigiPhysics.velocityX = 0.0f; luigiPhysics.velocityZ = 0.0f;
                    luigiPhysics.velocityY = 13.0f; luigiPhysics.isGrounded = false;
                }
            }
        } else {
            float deltaX = marioTransform.x - luigiTransform.x; float deltaZ = marioTransform.z - luigiTransform.z;
            float playerDist = std::sqrt(deltaX * deltaX + deltaZ * deltaZ);
            if (playerDist > 3.5f) {
                luigiPhysics.velocityX = (deltaX / playerDist) * luigiMovementSpeed;
                luigiPhysics.velocityZ = (deltaZ / playerDist) * luigiMovementSpeed;
                luigiTransform.rotationY = atan2(deltaX, deltaZ) * (180.0f / 3.14159265f);
            } else {
                luigiPhysics.velocityX = 0.0f; luigiPhysics.velocityZ = 0.0f;
            }
        }
    } else {
        luigiPhysics.velocityX = 0.0f; luigiPhysics.velocityZ = 0.0f;
    }

    luigiTransform.x += luigiPhysics.velocityX * deltaTime; luigiTransform.z += luigiPhysics.velocityZ * deltaTime;
    luigiPhysics.velocityY -= 20.0f * deltaTime; luigiTransform.y += luigiPhysics.velocityY * deltaTime;
    
    luigiPhysics.isGrounded = false;
    if (luigiTransform.y <= 0.5f) { luigiTransform.y = 0.5f; luigiPhysics.velocityY = 0.0f; luigiPhysics.isGrounded = true; }

    for (int bIdx = 0; bIdx < worldBlocks.size(); ++bIdx) {
        Resolve3DCollision(luigiTransform.x, luigiTransform.y, luigiTransform.z, luigiPhysics.velocityY, luigiPhysics.isGrounded, worldBlocks[bIdx], 0.35f, 0.45f, 0.35f, bIdx, eventQueue, currentMode, 2);
    }

    if (currentMode == PLAY_MODE) {
        if (damageCooldown > 0.0f) damageCooldown -= deltaTime;

        for (Entity e : enemyList) {
            EnemyAI& ai = ecs.getEnemyAI(e); if (!ai.isAlive) continue;
            Transform& enemyTransform = ecs.getTransform(e); PhysicsBody& enemyPhysics = ecs.getPhysicsBody(e);

            float distX = marioTransform.x - enemyTransform.x; float distZ = marioTransform.z - enemyTransform.z;
            float distToPlayer = std::sqrt((distX * distX) + (distZ * distZ));

            float avoidX = 0.0f; float avoidZ = 0.0f;
            for (const auto& block : worldBlocks) {
                if (block.type == ASSET_TRAP) {
                    float hdx = enemyTransform.x - block.x; float hdz = enemyTransform.z - block.z;
                    float hdist = std::sqrt(hdx*hdx + hdz*hdz);
                    if (hdist < 5.0f && hdist > 0.1f) { avoidX += (hdx / hdist) * 8.0f; avoidZ += (hdz / hdist) * 8.0f; }
                }
            }

            if (distToPlayer > 0.1f) {
                enemyTransform.x += ((distX / distToPlayer) * globalEnemySpeed + avoidX) * deltaTime;
                enemyTransform.z += ((distZ / distToPlayer) * globalEnemySpeed + avoidZ) * deltaTime;
            }

            enemyPhysics.velocityY -= 20.0f * deltaTime; enemyTransform.y += enemyPhysics.velocityY * deltaTime;
            enemyPhysics.isGrounded = false;
            if (enemyTransform.y <= 0.5f) { enemyTransform.y = 0.5f; enemyPhysics.velocityY = 0.0f; enemyPhysics.isGrounded = true; }

            for (int bIdx = 0; bIdx < worldBlocks.size(); ++bIdx) {
                bool enemyGroundedTemp = enemyPhysics.isGrounded;
                Resolve3DCollision(enemyTransform.x, enemyTransform.y, enemyTransform.z, enemyPhysics.velocityY, enemyGroundedTemp, worldBlocks[bIdx], 0.4f, 0.3f, 0.4f, bIdx, eventQueue, currentMode, 0);
                if (enemyGroundedTemp) enemyPhysics.isGrounded = true;
            }

            if (!enemyPhysics.isGrounded && enableEnemyJumping) {
                if (((static_cast<float>(rand()) / RAND_MAX) < (1.2f * deltaTime))) enemyPhysics.velocityY = 12.0f; 
            } else if (enableEnemyJumping && distToPlayer < 4.0f && marioTransform.y > enemyTransform.y + 1.0f) {
                if (((static_cast<float>(rand()) / RAND_MAX) < (2.0f * deltaTime))) enemyPhysics.velocityY = 12.0f; 
            }

            float sepX = 0.0f; float sepZ = 0.0f;
            std::vector<Entity> neighborsToAvoid = enableSpatialGrid ? spatialGrid.getNearbyEntities(enemyTransform.x, enemyTransform.z, 1.5f) : enemyList;
            for (Entity other : neighborsToAvoid) {
                if (other == e || !ecs.getEnemyAI(other).isAlive) continue;
                Transform& otherT = ecs.getTransform(other);
                float dX = enemyTransform.x - otherT.x; float dZ = enemyTransform.z - otherT.z; float dSq = dX*dX + dZ*dZ;
                if (dSq > 0.001f && dSq < 2.25f) { float dist = std::sqrt(dSq); sepX += (dX / dist); sepZ += (dZ / dist); }
            }
            enemyTransform.x += sepX * deltaTime * 3.5f; enemyTransform.z += sepZ * deltaTime * 3.5f;

            // RE-INJECTED: Player stomp and damage calculation node block
            if (distToPlayer < 1.0f && std::abs(marioTransform.y - enemyTransform.y) < 1.5f) {
                if (marioPhysics.velocityY < 0.0f && marioTransform.y > enemyTransform.y + 0.3f) {
                    ai.isAlive = false; 
                    marioPhysics.velocityY = 10.0f; // Triggers player bounce force
                } 
                else if (damageCooldown <= 0.0f && !isPlayerInvulnerable) { 
                    eventQueue.push_back({ EVENT_PLAYER_DAMAGE });
                }
            }

            float deltaXLuigi = luigiTransform.x - enemyTransform.x; float deltaZLuigi = luigiTransform.z - enemyTransform.z;
            float distToLuigi = std::sqrt(deltaXLuigi * deltaXLuigi + deltaZLuigi * deltaZLuigi);
            if (distToLuigi < 1.0f && std::abs(luigiTransform.y - enemyTransform.y) < 1.5f) {
                if (luigiPhysics.velocityY < 0.0f && luigiTransform.y > enemyTransform.y + 0.3f) {
                    ai.isAlive = false; luigiPhysics.velocityY = 10.0f;
                }
            }
        }
    }
}