#include "Engine/Window.h"
#include "Engine/Core/ECS.h"
#include "Engine/Core/SpatialGrid.h"
#include <GLFW/glfw3.h>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>
#include <iostream>

// --- IMGUI INCLUDES ---
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

enum GameState { MAIN_MENU, PLAYING, PAUSED, STAGE_COMPLETED };
enum EngineMode { EDIT_MODE, PLAY_MODE };

enum AssetType { ASSET_BLOCK, ASSET_ROCK, ASSET_HILL, ASSET_TREE, ASSET_TRAP };

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

enum EventType { EVENT_SPAWN_ENEMY, EVENT_SPAWN_ASSET, EVENT_CLEAR_BLOCKS, EVENT_PURGE_ENEMIES, EVENT_PLAYER_DAMAGE };

struct EngineEvent {
    EventType type;
    AssetType assetType;
    float paramX = 0.0f;
    float paramY = 0.0f;
    float paramZ = 0.0f;
    int paramValue = 0;
};

// --- HIGH PERFORMANCE PROCEDURAL NOISE SYSTEM ---
float GetProceduralNoise(float x, float z, float frequency) {
    float n = sin(x * frequency) * cos(z * frequency) + sin(x * frequency * 2.5f) * sin(z * frequency * 1.8f);
    return (n + 2.0f) / 4.0f; 
}

// --- DYNAMIC TEXTURE PRIMITIVE DRAWERS ---
void DrawProceduralCube(float baseR, float baseG, float baseB, float noiseFreq, bool isGrassTop = false) {
    glBegin(GL_QUADS);
    
    // TOP FACE
    glNormal3f(0.0f, 1.0f, 0.0f);
    float r = baseR; float g = baseG; float b = baseB;
    if (isGrassTop) { r = 0.12f; g = 0.52f; b = 0.18f; } 
    glColor3f(r * 0.9f, g * 0.9f, b * 0.9f);
    glVertex3f(-0.5f,  0.5f,  0.5f); glVertex3f( 0.5f,  0.5f,  0.5f); glVertex3f( 0.5f,  0.5f, -0.5f); glVertex3f(-0.5f,  0.5f, -0.5f);
    
    // FRONT FACE
    glNormal3f(0.0f, 0.0f, 1.0f); glColor3f(baseR, baseG, baseB);
    glVertex3f(-0.5f, -0.5f,  0.5f); glVertex3f( 0.5f, -0.5f,  0.5f); glVertex3f( 0.5f,  0.5f,  0.5f); glVertex3f(-0.5f,  0.5f,  0.5f);
    
    // BACK FACE
    glNormal3f(0.0f, 0.0f, -1.0f); glColor3f(baseR * 0.85f, baseG * 0.85f, baseB * 0.85f);
    glVertex3f( 0.5f, -0.5f, -0.5f); glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f(-0.5f,  0.5f, -0.5f); glVertex3f( 0.5f,  0.5f, -0.5f);
    
    // LEFT FACE
    glNormal3f(-1.0f, 0.0f, 0.0f); glColor3f(baseR * 0.75f, baseG * 0.75f, baseB * 0.75f);
    glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f(-0.5f, -0.5f,  0.5f); glVertex3f(-0.5f,  0.5f,  0.5f); glVertex3f(-0.5f,  0.5f, -0.5f);
    
    // RIGHT FACE
    glNormal3f(1.0f, 0.0f, 0.0f); glColor3f(baseR * 0.8f, baseG * 0.8f, baseB * 0.8f);
    glVertex3f( 0.5f, -0.5f,  0.5f); glVertex3f( 0.5f, -0.5f, -0.5f); glVertex3f( 0.5f, -0.5f,  0.5f); glVertex3f( 0.5f,  0.5f,  0.5f);
    
    // BOTTOM FACE
    glNormal3f(0.0f, -1.0f, 0.0f); glColor3f(baseR * 0.6f, baseG * 0.6f, baseB * 0.6f);
    glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f( 0.5f, -0.5f, -0.5f); glVertex3f( 0.5f, -0.5f,  0.5f); glVertex3f(-0.5f, -0.5f,  0.5f);
    glEnd();
}

void DrawPlayer(float r, float g, float b) {
    glPushMatrix(); glTranslatef(0.0f, 0.1f, 0.0f); glScalef(0.7f, 0.9f, 0.7f); DrawProceduralCube(r, g, b, 0.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, 0.25f, 0.35f); glScalef(0.5f, 0.3f, 0.1f); DrawProceduralCube(0.9f, 0.9f, 1.0f, 0.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.15f, -0.35f, 0.0f); glScalef(0.25f, 0.3f, 0.25f); DrawProceduralCube(r, g, b, 0.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.15f, -0.35f, 0.0f); glScalef(0.25f, 0.3f, 0.25f); DrawProceduralCube(r, g, b, 0.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, 0.55f, 0.0f); glScalef(0.75f, 0.05f, 0.75f); DrawProceduralCube(r*1.1f, g, b, 0.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, 0.6f, 0.0f); glScalef(0.6f, 0.1f, 0.6f); DrawProceduralCube(1.0f, 1.0f, 1.0f, 0.0f); glPopMatrix();
}

void DrawEnemy(float r, float g, float b) {
    glPushMatrix(); glTranslatef(0.0f, 0.0f, 0.0f); glScalef(0.8f, 0.6f, 0.8f); DrawProceduralCube(r*0.6f, g*0.6f, b*0.6f, 0.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, 0.4f, 0.0f); glScalef(0.9f, 0.3f, 0.9f); DrawProceduralCube(r, g, b, 0.0f); glPopMatrix();
}

void DrawPlatform() {
    float size = 2.0f; glBegin(GL_QUADS); glNormal3f(0.0f, 1.0f, 0.0f);
    for (float x = -100.0f; x < 100.0f; x += size) {
        for (float z = -100.0f; z < 100.0f; z += size) {
            float noise = GetProceduralNoise(x, z, 0.35f);
            float r = 0.10f + (noise * 0.06f); float g = 0.48f + (noise * 0.12f); float b = 0.16f + (noise * 0.04f);
            glColor3f(r, g, b);
            glVertex3f(x, 0.0f, z); glVertex3f(x + size, 0.0f, z); glVertex3f(x + size, 0.0f, z + size); glVertex3f(x, 0.0f, z + size);
        }
    }
    glEnd();
}

void DrawCustomAsset(const EnvironmentBlock& block) {
    if (block.type == ASSET_BLOCK || block.type == ASSET_ROCK) {
        glPushMatrix(); glTranslatef(block.x, block.y, block.z); glScalef(block.scaleX, block.scaleY, block.scaleZ); DrawProceduralCube(0.42f, 0.44f, 0.46f, 1.2f); glPopMatrix();
    } 
    else if (block.type == ASSET_HILL) {
        for (int i = 0; i < 3; ++i) {
            glPushMatrix();
            float stepY = block.y + (i * 0.4f); float stepScaleX = block.scaleX - (i * 1.0f); float stepScaleZ = block.scaleZ - (i * 1.0f);
            glTranslatef(block.x, stepY, block.z); glScalef(stepScaleX, 0.4f, stepScaleZ);
            bool isTopGrass = (i == 2);
            DrawProceduralCube(0.36f, 0.24f, 0.14f, 2.5f, isTopGrass); 
            glPopMatrix();
        }
    } 
    else if (block.type == ASSET_TREE) {
        glPushMatrix(); glTranslatef(block.x, block.y + 0.75f, block.z); glScalef(0.4f, 1.5f, 0.4f); DrawProceduralCube(0.32f, 0.20f, 0.08f, 0.0f); glPopMatrix();
        glPushMatrix(); glTranslatef(block.x, block.y + 2.0f, block.z); glScalef(1.6f, 1.0f, 1.6f); DrawProceduralCube(0.08f, 0.45f, 0.12f, 3.0f); glPopMatrix();
    }
    else if (block.type == ASSET_TRAP) {
        for (float dx = -0.8f; dx <= 0.8f; dx += 0.4f) {
            for (float dz = -0.8f; dz <= 0.8f; dz += 0.4f) {
                glPushMatrix(); glTranslatef(block.x + dx, 0.15f, block.z + dz); glScalef(0.12f, 0.35f, 0.12f); DrawProceduralCube(0.28f, 0.32f, 0.15f, 0.0f); glPopMatrix();
            }
        }
    }
}

void DrawShadow(float radius) {
    glDisable(GL_LIGHTING); glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); glDepthMask(GL_FALSE); glColor4f(0.0f, 0.0f, 0.0f, 0.4f); 
    glBegin(GL_QUADS); glVertex3f(-radius, 0.02f, radius); glVertex3f(radius, 0.02f, radius); glVertex3f(radius, 0.02f, -radius); glVertex3f(-radius, 0.02f, -radius); glEnd();
    glDepthMask(GL_TRUE); glDisable(GL_BLEND); glEnable(GL_LIGHTING);
}

void Resolve3DCollision(float& posX, float& posY, float& posZ, float& velY, bool& grounded, const EnvironmentBlock& block, float extX, float extY, float extZ) {
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

int main() {
    srand(static_cast<unsigned int>(time(0))); 
    Window myWindow(1440, 900, "VIE-Project Environment Builder Engine");
    Registry ecs;

    IMGUI_CHECKVERSION(); ImGui::CreateContext(); ImGuiIO& io = ImGui::GetIO(); (void)io; ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(myWindow.getRawWindow(), true); ImGui_ImplOpenGL3_Init("#version 120");

    SpatialGrid spatialGrid(4.0f);
    std::vector<EnvironmentBlock> worldBlocks;
    std::vector<EngineEvent> eventQueue;

    std::vector<EnvironmentBlock> backupWorldBlocks;
    std::vector<EnemySnapshot> backupEnemyList;
    float backupPlayerX = 0.0f, backupPlayerY = 0.5f, backupPlayerZ = 0.0f;

    float enemyColor[3]     = { 0.54f, 0.27f, 0.07f }; 
    float characterColor[3] = { 0.90f, 0.10f, 0.10f }; 

    float globalEnemySpeed = 4.0f; float playerMovementSpeed = 15.0f; bool enableSpatialGrid = true; bool enableEnemyJumping = false; 
    bool isPlayerInvulnerable = true; int playerMaxHealth = 10; int playerHealth = 10; 

    Entity mario = ecs.createEntity(); std::vector<Entity> enemyList;
    GameState currentState = MAIN_MENU; EngineMode currentMode = EDIT_MODE; float damageCooldown = 0.0f; 

    glEnable(GL_LIGHTING); glEnable(GL_LIGHT0); glEnable(GL_COLOR_MATERIAL); glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    GLfloat light_position[] = { 10.0f, 50.0f, 20.0f, 1.0f }; GLfloat light_ambient[] = { 0.4f, 0.4f, 0.4f, 1.0f }; GLfloat light_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f }; 
    glLightfv(GL_LIGHT0, GL_POSITION, light_position); glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient); glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);

    auto handleSpawnEnemy = [&](int count) {
        for (int i = 0; i < count; i++) {
            Entity e = ecs.createEntity(); Transform& t = ecs.getTransform(e); EnemyAI& ai = ecs.getEnemyAI(e);
            t.x = (rand() % 80) - 40.0f; t.z = (rand() % 80) - 40.0f; t.y = 0.5f; 
            if (std::abs(t.x) < 10.0f && std::abs(t.z) < 10.0f) { t.x += 15.0f; } 
            ai.spawnX = t.x; ai.spawnZ = t.z; ai.isAlive = true; enemyList.push_back(e);
        }
    };

    auto enterSandboxEnvironment = [&]() {
        playerMaxHealth = 10; playerHealth = 10; damageCooldown = 0.0f; playerMovementSpeed = 15.0f; globalEnemySpeed = 4.0f;
        worldBlocks.clear(); eventQueue.push_back({ EVENT_CLEAR_BLOCKS }); eventQueue.push_back({ EVENT_PURGE_ENEMIES });
        Transform& mt = ecs.getTransform(mario); mt.x = 0.0f; mt.y = 0.5f; mt.z = 0.0f; mt.rotationY = 0.0f;
        PhysicsBody& mp = ecs.getPhysicsBody(mario); mp.velocityX = 0.0f; mp.velocityY = 0.0f; mp.velocityZ = 0.0f;
        currentMode = EDIT_MODE; currentState = PLAYING; 
    };

    float lastTime = myWindow.getTime(); glEnable(GL_DEPTH_TEST);

    while (!myWindow.shouldClose()) {
        float currentTime = myWindow.getTime(); float deltaTime = currentTime - lastTime; lastTime = currentTime;
        myWindow.pollEvents();
        
        Transform& marioTransform = ecs.getTransform(mario); PhysicsBody& marioPhysics = ecs.getPhysicsBody(mario);

        static bool escPressedLastFrame = false; bool escPressedThisFrame = myWindow.isKeyPressed(GLFW_KEY_ESCAPE);
        if (escPressedThisFrame && !escPressedLastFrame) { if (currentState == PLAYING) currentState = PAUSED; else if (currentState == PAUSED) currentState = PLAYING; }
        escPressedLastFrame = escPressedThisFrame;

        ImGui_ImplOpenGL3_NewFrame(); ImGui_ImplGlfw_NewFrame(); ImGui::NewFrame();

        if (currentState == PLAYING) {
            spatialGrid.clear();
            if (enableSpatialGrid) {
                for (Entity e : enemyList) { if (ecs.getEnemyAI(e).isAlive) spatialGrid.insert(e, ecs.getTransform(e).x, ecs.getTransform(e).z); }
            }

            // --- UNIVERSAL PLAYER MOVEMENT TRACKING LOOP ---
            marioPhysics.velocityX = 0.0f; marioPhysics.velocityZ = 0.0f; 
            if (myWindow.isKeyPressed(GLFW_KEY_D)) { marioPhysics.velocityX = playerMovementSpeed; marioTransform.rotationY = 90.0f; }
            if (myWindow.isKeyPressed(GLFW_KEY_A)) { marioPhysics.velocityX = -playerMovementSpeed; marioTransform.rotationY = -90.0f; }
            if (myWindow.isKeyPressed(GLFW_KEY_S)) { marioPhysics.velocityZ = playerMovementSpeed; marioTransform.rotationY = 0.0f; }
            if (myWindow.isKeyPressed(GLFW_KEY_W)) { marioPhysics.velocityZ = -playerMovementSpeed; marioTransform.rotationY = 180.0f; }
            
            marioTransform.x += marioPhysics.velocityX * deltaTime; marioTransform.z += marioPhysics.velocityZ * deltaTime;

            // --- GLOBAL VERTICAL PHYSICS & GRAVITY TRACKING ---
            // Enabled globally across all runtime modes to allow jump testing in editor canvas spaces
            if (marioPhysics.velocityY > 0 && !myWindow.isKeyPressed(GLFW_KEY_SPACE)) marioPhysics.velocityY -= 50.0f * deltaTime; 
            else marioPhysics.velocityY -= 20.0f * deltaTime; 
            marioTransform.y += marioPhysics.velocityY * deltaTime;

            marioPhysics.isGrounded = false;
            if (marioTransform.y <= 0.5f) { marioTransform.y = 0.5f; marioPhysics.velocityY = 0.0f; marioPhysics.isGrounded = true; }

            for (const auto& block : worldBlocks) {
                Resolve3DCollision(marioTransform.x, marioTransform.y, marioTransform.z, marioPhysics.velocityY, marioPhysics.isGrounded, block, 0.35f, 0.45f, 0.35f);
                if (currentMode == PLAY_MODE && block.type == ASSET_TRAP && std::abs(marioTransform.x - block.x) < 1.4f && std::abs(marioTransform.z - block.z) < 1.4f) {
                    if (damageCooldown <= 0.0f && !isPlayerInvulnerable) eventQueue.push_back({ EVENT_PLAYER_DAMAGE });
                }
            }

            if (marioTransform.x < -99.5f) marioTransform.x = -99.5f; if (marioTransform.x >  99.5f) marioTransform.x =  99.5f;
            if (marioTransform.z < -99.5f) marioTransform.z = -99.5f; if (marioTransform.z >  99.5f) marioTransform.z =  99.5f;

            if (myWindow.isKeyPressed(GLFW_KEY_SPACE) && marioPhysics.isGrounded) { marioPhysics.velocityY = 12.0f; marioPhysics.isGrounded = false; }

            // --- SIMULATION MODE SUB-SYSTEM LOOP ---
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

                    for (const auto& block : worldBlocks) {
                        bool enemyGroundedTemp = enemyPhysics.isGrounded;
                        Resolve3DCollision(enemyTransform.x, enemyTransform.y, enemyTransform.z, enemyPhysics.velocityY, enemyGroundedTemp, block, 0.4f, 0.3f, 0.4f);
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

                    if (distToPlayer < 1.0f && std::abs(marioTransform.y - enemyTransform.y) < 1.5f) {
                        if (marioPhysics.velocityY < 0.0f && marioTransform.y > enemyTransform.y + 0.3f) {
                            ai.isAlive = false; marioPhysics.velocityY = 10.0f; 
                        } 
                        else if (damageCooldown <= 0.0f && !isPlayerInvulnerable) { 
                            eventQueue.push_back({ EVENT_PLAYER_DAMAGE });
                        }
                    }
                }
                
                if (playerHealth <= 0) {
                    currentMode = EDIT_MODE; worldBlocks = backupWorldBlocks;
                    marioTransform.x = backupPlayerX; marioTransform.y = backupPlayerY; marioTransform.z = backupPlayerZ;
                    marioPhysics.velocityX = 0.0f; marioPhysics.velocityY = 0.0f; marioPhysics.velocityZ = 0.0f;
                    playerHealth = playerMaxHealth;
                    for (Entity e : enemyList) { ecs.getEnemyAI(e).isAlive = false; } enemyList.clear();
                    for (const auto& sn : backupEnemyList) {
                        Entity e = ecs.createEntity(); Transform& t = ecs.getTransform(e); EnemyAI& ai = ecs.getEnemyAI(e);
                        t.x = sn.spawnX; t.z = sn.spawnZ; t.y = 0.5f; ai.isAlive = true; enemyList.push_back(e);
                    }
                }
            }
        }

        // --- IMGUI ENVIRONMENTAL EDITOR PANEL ---
        if (currentState == MAIN_MENU) {
            ImGui::SetNextWindowPos(ImVec2(1440.0f / 2.0f, 900.0f / 2.0f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::Begin("Main Menu", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);
            ImGui::SetWindowFontScale(2.5f); ImGui::Text("VIRTUAL ENVIRONMENT MAKER"); ImGui::SetWindowFontScale(1.5f);
            ImGui::Dummy(ImVec2(0.0f, 20.0f)); 
            if (ImGui::Button("Enter Sandbox Creator Platform", ImVec2(450, 80))) { enterSandboxEnvironment(); } ImGui::Dummy(ImVec2(0.0f, 5.0f)); 
            if (ImGui::Button("Quit to Desktop", ImVec2(450, 60))) { glfwSetWindowShouldClose(myWindow.getRawWindow(), true); }
            ImGui::End();
        }

        if (currentState == PAUSED) {
            ImGui::SetNextWindowPos(ImVec2(1440.0f / 2.0f, 900.0f / 2.0f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::Begin("Pause Menu", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);
            ImGui::SetWindowFontScale(2.5f); ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "ENVIRONMENT PAUSED"); ImGui::SetWindowFontScale(1.5f);
            ImGui::Dummy(ImVec2(0.0f, 20.0f));
            if (ImGui::Button("Resume Creator", ImVec2(300, 50))) { currentState = PLAYING; } ImGui::Dummy(ImVec2(0.0f, 5.0f)); 
            if (ImGui::Button("Return to Main Menu", ImVec2(300, 50))) { currentState = MAIN_MENU; }
            ImGui::End();
        }

        if (currentState == PLAYING || currentState == PAUSED) {
            ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_Always);
            ImGui::Begin("StatusOverlay", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs);
            ImGui::SetWindowFontScale(2.0f);
            if (currentMode == EDIT_MODE) ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.0f, 1.0f), "MODE: ENVIRONMENT EDITOR (FROZEN)");
            else ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "MODE: SIMULATION PLAYTEST (ACTIVE)");
            ImGui::End();

            if (currentMode == PLAY_MODE) {
                ImGui::SetNextWindowPos(ImVec2(20.0f, 80.0f), ImGuiCond_Always);
                ImGui::Begin("Playtest Status Context", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);
                ImGui::SetWindowFontScale(1.2f);
                // FIXED STRINGS: Emojis stripped out to fix ?? rendering bugs
                if (ImGui::Button("Stop Playtest and Reset Layout", ImVec2(320, 45))) {
                    currentMode = EDIT_MODE; worldBlocks = backupWorldBlocks;
                    marioTransform.x = backupPlayerX; marioTransform.y = backupPlayerY; marioTransform.z = backupPlayerZ;
                    marioPhysics.velocityX = 0.0f; marioPhysics.velocityY = 0.0f; marioPhysics.velocityZ = 0.0f;
                    playerHealth = playerMaxHealth;
                    for (Entity e : enemyList) { ecs.getEnemyAI(e).isAlive = false; } enemyList.clear();
                    for (const auto& sn : backupEnemyList) {
                        Entity e = ecs.createEntity(); Transform& t = ecs.getTransform(e); EnemyAI& ai = ecs.getEnemyAI(e);
                        t.x = sn.spawnX; t.z = sn.spawnZ; t.y = 0.5f; ai.isAlive = true; enemyList.push_back(e);
                    }
                }
                ImGui::End();
            }

            if (currentMode == EDIT_MODE) {
                ImGui::SetNextWindowPos(ImVec2(20.0f, 80.0f), ImGuiCond_Once); ImGui::SetNextWindowSize(ImVec2(480.0f, 780.0f), ImGuiCond_Once);
                ImGui::Begin("Environment Control Panel"); ImGui::SetWindowFontScale(1.15f);
                
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Execution Controller Matrix"); ImGui::Separator();
                if (ImGui::Button("Start Playtest Simulation", ImVec2(-1, 45))) {
                    currentMode = PLAY_MODE; backupWorldBlocks = worldBlocks;
                    backupPlayerX = marioTransform.x; backupPlayerY = marioTransform.y; backupPlayerZ = marioTransform.z;
                    backupEnemyList.clear();
                    for (Entity e : enemyList) {
                        if (ecs.getEnemyAI(e).isAlive) backupEnemyList.push_back({ ecs.getTransform(e).x, ecs.getTransform(e).z });
                    }
                }
                ImGui::Dummy(ImVec2(0, 10));

                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Simulation Target Spawning"); ImGui::Separator();
                if (ImGui::Button("Swarm x1", ImVec2(120, 32))) { eventQueue.push_back({ EVENT_SPAWN_ENEMY, ASSET_BLOCK, 0, 0, 0, 1 }); } ImGui::SameLine();
                if (ImGui::Button("Swarm x10", ImVec2(130, 32))) { eventQueue.push_back({ EVENT_SPAWN_ENEMY, ASSET_BLOCK, 0, 0, 0, 10 }); } ImGui::SameLine();
                if (ImGui::Button("Overload x1000", ImVec2(170, 32))) { eventQueue.push_back({ EVENT_SPAWN_ENEMY, ASSET_BLOCK, 0, 0, 0, 1000 }); }
                
                float rad = marioTransform.rotationY * (3.14159265f / 180.0f);
                float pX = marioTransform.x + sin(rad) * 4.0f; float pZ = marioTransform.z + cos(rad) * 4.0f;

                ImGui::Dummy(ImVec2(0, 5)); ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Visual Palette Generation Toolbar"); ImGui::Separator();
                if (ImGui::Button("Rock Block", ImVec2(140, 40))) { eventQueue.push_back({ EVENT_SPAWN_ASSET, ASSET_ROCK, pX, 0.5f, pZ, 0 }); } ImGui::SameLine();
                if (ImGui::Button("Small Hill", ImVec2(140, 40))) { eventQueue.push_back({ EVENT_SPAWN_ASSET, ASSET_HILL, pX, 0.2f, pZ, 0 }); } ImGui::SameLine();
                if (ImGui::Button("Wood Tree", ImVec2(140, 40))) { eventQueue.push_back({ EVENT_SPAWN_ASSET, ASSET_TREE, pX, 0.0f, pZ, 0 }); }
                if (ImGui::Button("Hazard Vine Trap", ImVec2(-1, 35))) { eventQueue.push_back({ EVENT_SPAWN_ASSET, ASSET_TRAP, pX, 0.0f, pZ, 0 }); }
                
                if (ImGui::Button("Clear Placed Objects", ImVec2(215, 30))) { eventQueue.push_back({ EVENT_CLEAR_BLOCKS }); } ImGui::SameLine();
                if (ImGui::Button("Purge All Enemies", ImVec2(220, 30))) { eventQueue.push_back({ EVENT_PURGE_ENEMIES }); }

                ImGui::Dummy(ImVec2(0, 10)); ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Player Health Sandbox Matrices"); ImGui::Separator();
                ImGui::Checkbox("Player Invulnerability Toggle", &isPlayerInvulnerable);
                
                ImGui::PushItemWidth(-170.0f);
                if(ImGui::SliderInt("Max Health Capacity", &playerMaxHealth, 1, 10)) { playerHealth = playerMaxHealth; }
                ImGui::SliderInt("Current Player Health", &playerHealth, 0, playerMaxHealth);
                ImGui::PopItemWidth();

                ImGui::Dummy(ImVec2(0, 10)); ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Material Vector Color Field"); ImGui::Separator();
                ImGui::PushItemWidth(-170.0f); ImGui::ColorEdit3("Swarm Enemy Model", enemyColor); ImGui::ColorEdit3("Player Avatar Model", characterColor); ImGui::PopItemWidth();

                ImGui::Dummy(ImVec2(0, 10)); ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Global Simulation Sliders"); ImGui::Separator();
                ImGui::PushItemWidth(-170.0f);
                ImGui::SliderFloat("Player Movement Speed", &playerMovementSpeed, 5.0f, 40.0f, "%.1f units/s");
                ImGui::SliderFloat("Enemy Tracking Speed", &globalEnemySpeed, 0.0f, 25.0f, "%.1f units/s");
                ImGui::PopItemWidth();
                ImGui::Checkbox("Enable Enemy Jumping Force", &enableEnemyJumping);

                ImGui::Dummy(ImVec2(0, 10)); ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Optimization Toggles"); ImGui::Separator();
                ImGui::Checkbox("Enable Spatial Hash Partitioning Grid", &enableSpatialGrid);
                
                int activeEnemies = 0; for(Entity e : enemyList) { if(ecs.getEnemyAI(e).isAlive) activeEnemies++; }
                ImGui::Dummy(ImVec2(0, 10)); ImGui::Separator();
                ImGui::Text("Active Enemies: %d | Assets Placed: %lu", activeEnemies, worldBlocks.size()); ImGui::End();
            }

            ImGui::SetNextWindowPos(ImVec2(1420.0f, 850.0f), ImGuiCond_Always, ImVec2(1.0f, 1.0f));
            ImGui::Begin("InstructionsOverlay", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs);
            ImGui::SetWindowFontScale(1.3f);
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.6f), "Drive character in EDIT mode with WASD to build across the map canvas");
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.6f), "Press [ESC] to view configuration system parameters"); ImGui::End();

            ImGui::SetNextWindowPos(ImVec2(1420.0f, 20.0f), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
            ImGui::Begin("FPSOverlay", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs);
            ImGui::SetWindowFontScale(1.5f); ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "FPS: %.1f", io.Framerate); ImGui::End();
        }

        // --- ASSET EVENT DISPATCHER CORE ---
        for (const auto& ev : eventQueue) {
            switch (ev.type) {
                case EVENT_SPAWN_ENEMY:
                    handleSpawnEnemy(ev.paramValue);
                    break;
                case EVENT_SPAWN_ASSET:
                    if (ev.assetType == ASSET_ROCK) {
                        worldBlocks.push_back({ ASSET_ROCK, ev.paramX, ev.paramY, ev.paramZ, 2.2f, 1.2f, 2.2f, 0.0f, 0.0f, 0.0f, true });
                    } else if (ev.assetType == ASSET_HILL) {
                        worldBlocks.push_back({ ASSET_HILL, ev.paramX, ev.paramY, ev.paramZ, 5.0f, 0.5f, 5.0f, 0.0f, 0.0f, 0.0f, true });
                    } else if (ev.assetType == ASSET_TREE) {
                        worldBlocks.push_back({ ASSET_TREE, ev.paramX, ev.paramY, ev.paramZ, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, false });
                    } else if (ev.assetType == ASSET_TRAP) {
                        worldBlocks.push_back({ ASSET_TRAP, ev.paramX, ev.paramY, ev.paramZ, 2.5f, 0.1f, 2.5f, 0.0f, 0.0f, 0.0f, true });
                    }
                    break;
                case EVENT_CLEAR_BLOCKS:
                    worldBlocks.clear();
                    break;
                case EVENT_PURGE_ENEMIES:
                    for (Entity e : enemyList) { ecs.getEnemyAI(e).isAlive = false; }
                    enemyList.clear();
                    break;
                case EVENT_PLAYER_DAMAGE:
                    playerHealth -= 1; damageCooldown = 1.5f;
                    if (playerHealth <= 0) playerHealth = 0;
                    break;
                default: break;
            }
        }
        eventQueue.clear(); 

        // --- RENDER UPDATE ---
        glClearColor(0.12f, 0.28f, 0.44f, 1.0f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (currentState != MAIN_MENU) {
            glMatrixMode(GL_PROJECTION); glLoadIdentity(); glFrustum(-1.6, 1.6, -1.0, 1.0, 2.0, 200.0); 
            glMatrixMode(GL_MODELVIEW); glLoadIdentity(); glRotatef(25.0f, 1.0f, 0.0f, 0.0f); glTranslatef(-marioTransform.x, -8.0f, -18.0f - marioTransform.z); 

            // Shadows
            glPushMatrix(); glTranslatef(marioTransform.x, 0.01f, marioTransform.z); DrawShadow(0.6f); glPopMatrix();
            for (Entity e : enemyList) { if (ecs.getEnemyAI(e).isAlive) { glPushMatrix(); glTranslatef(ecs.getTransform(e).x, 0.01f, ecs.getTransform(e).z); DrawShadow(0.7f); glPopMatrix(); } }

            glEnable(GL_LIGHTING);
            DrawPlatform(); 
            
            for (const auto& block : worldBlocks) { DrawCustomAsset(block); }
            for (Entity e : enemyList) { if (ecs.getEnemyAI(e).isAlive) { glPushMatrix(); glTranslatef(ecs.getTransform(e).x, ecs.getTransform(e).y, ecs.getTransform(e).z); DrawEnemy(enemyColor[0], enemyColor[1], enemyColor[2]); glPopMatrix(); } }

            glPushMatrix(); glTranslatef(marioTransform.x, marioTransform.y, marioTransform.z); glRotatef(marioTransform.rotationY, 0.0f, 1.0f, 0.0f); 
            glPushMatrix(); glTranslatef(0.0f, 0.4f, 0.5f); glScalef(0.1f, 0.1f, 0.1f); DrawProceduralCube(1.0f, 0.0f, 0.0f, 0.0f); glPopMatrix(); 
            DrawPlayer(characterColor[0], characterColor[1], characterColor[2]); glPopMatrix();
            glDisable(GL_LIGHTING);

            glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity(); glOrtho(0.0, 1440.0, 900.0, 0.0, -1.0, 1.0); 
            glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity(); glDisable(GL_DEPTH_TEST); 

            float barWidth = 400.0f; float barHeight = 30.0f; float startX = (1440.0f / 2.0f) - (barWidth / 2.0f); float startY = 900.0f - barHeight - 20.0f; 
            glBegin(GL_QUADS); glColor3f(0.3f, 0.0f, 0.0f); glVertex2f(startX, startY); glVertex2f(startX + barWidth, startY); glVertex2f(startX + barWidth, startY + barHeight); glVertex2f(startX, startY + barHeight); glEnd();
            glBegin(GL_QUADS); glColor3f(0.0f, 1.0f, 0.0f);
            
            float healthRatio = (playerMaxHealth > 0) ? ((float)playerHealth / (float)playerMaxHealth) : 0.0f;
            float healthWidth = barWidth * healthRatio;
            if (healthWidth > 0.0f) { glVertex2f(startX, startY); glVertex2f(startX + healthWidth, startY); glVertex2f(startX + healthWidth, startY + barHeight); glVertex2f(startX, startY + barHeight); }
            glEnd();

            glEnable(GL_DEPTH_TEST); glMatrixMode(GL_MODELVIEW); glPopMatrix(); glMatrixMode(GL_PROJECTION); glPopMatrix();
        }

        ImGui::Render(); ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); myWindow.swapBuffers();
    }

    ImGui_ImplOpenGL3_Shutdown(); ImGui_ImplGlfw_Shutdown(); ImGui::DestroyContext(); return 0;
}