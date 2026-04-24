#pragma once
#include <cstdint>
#include <vector>

using Entity = uint32_t;

struct Transform {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float rotationY = 0.0f; 
};

struct PhysicsBody {
    float velocityX = 0.0f;
    float velocityY = 0.0f;
    float velocityZ = 0.0f;
    bool isGrounded = false;
};

// NEW: Data needed for our autonomous agents
struct EnemyAI {
    float spawnX = 0.0f;
    float spawnZ = 0.0f;
    bool isAlive = false; // Only true for actual enemy entities
};

class Registry {
public:
    Entity createEntity() {
        Entity newEntity = m_nextEntityID++;
        if (newEntity >= m_transforms.size()) {
            m_transforms.resize(newEntity + 100); 
            m_physicsBodies.resize(newEntity + 100);
            m_enemyAIs.resize(newEntity + 100); // Resize our new AI array
        }
        return newEntity;
    }

    Transform& getTransform(Entity e) { return m_transforms[e]; }
    PhysicsBody& getPhysicsBody(Entity e) { return m_physicsBodies[e]; }
    EnemyAI& getEnemyAI(Entity e) { return m_enemyAIs[e]; }

private:
    Entity m_nextEntityID = 0;
    std::vector<Transform> m_transforms;
    std::vector<PhysicsBody> m_physicsBodies;
    std::vector<EnemyAI> m_enemyAIs;
};