#include "Engine/Window.h"
#include "Engine/Core/ECS.h"
#include "Engine/Core/SpatialGrid.h"
#include "Engine/GameTypes.h"
#include "Engine/PhysicsEngine.h"
#include "Engine/RenderPipeline.h"
#include "Engine/FileHandler.h"
#include "Engine/BehaviorSystem.h"
#include "Engine/EventSystem.h"
#include <GLFW/glfw3.h>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>
#include <iostream>
#include <algorithm>

// --- IMGUI INCLUDES ---
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

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
    float backupLuigiX = 2.0f, backupLuigiY = 0.5f, backupLuigiZ = 0.0f;

    float enemyColor[3]     = { 0.54f, 0.27f, 0.07f }; 
    float characterColor[3] = { 0.90f, 0.10f, 0.10f }; 

    float globalEnemySpeed = 4.0f; float playerMovementSpeed = 15.0f; bool enableSpatialGrid = true; bool enableEnemyJumping = false; 
    bool isPlayerInvulnerable = true; int playerMaxHealth = 10; int playerHealth = 10; 

    // --- GUN SYSTEM SANDBOX STATE VECTORS ---
    bool isGunEnabled = false; int playerGunLevel = 1; float shootingCooldown = 0.0f;
    float weaponMessageTimer = 0.0f; 
    std::vector<Projectile> bulletList;
    bulletList.reserve(1000);

    // --- COOPERATIVE ALLY PARAMETERS ---
    bool luigiHasGun = false; float luigiShootingCooldown = 0.0f; float luigiMovementSpeed = 11.0f;
    bool luigiIsHelping = false; float luigiMessageTimer = 0.0f; std::string luigiMessage = "";

    // --- CONFIG SYSTEM CONFIGURATION STATE METRIC TARGETS ---
    MenuSubState currentMenuSubState = SUB_MAIN;
    SaveSubState currentSaveSubState = SAVE_INACTIVE;
    int pendingActiveTargetSlot = -1;

    Entity mario = ecs.createEntity(); 
    Entity luigi = ecs.createEntity(); 
    std::vector<Entity> enemyList;
    
    GameState currentState = MAIN_MENU; EngineMode currentMode = EDIT_MODE; float damageCooldown = 0.0f; 

    glEnable(GL_LIGHTING); glEnable(GL_LIGHT0); glEnable(GL_COLOR_MATERIAL); glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    GLfloat light_position[] = { 10.0f, 50.0f, 20.0f, 1.0f }; GLfloat light_ambient[] = { 0.4f, 0.4f, 0.4f, 1.0f }; GLfloat light_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f }; 
    glLightfv(GL_LIGHT0, GL_POSITION, light_position); glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient); glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);

    auto handleSpawnEnemy = [&](int count) {
        const int MAX_SAFE_CEILING = 3000;
        int activeAliveCount = 0;
        for (Entity e : enemyList) { if (ecs.getEnemyAI(e).isAlive) activeAliveCount++; }
        int regulatedCount = count;
        if (activeAliveCount + regulatedCount > MAX_SAFE_CEILING) { regulatedCount = MAX_SAFE_CEILING - activeAliveCount; }
        for (int i = 0; i < regulatedCount; i++) {
            Entity e = ecs.createEntity(); Transform& t = ecs.getTransform(e); EnemyAI& ai = ecs.getEnemyAI(e);
            t.x = (rand() % 80) - 40.0f; t.z = (rand() % 80) - 40.0f; t.y = 0.5f; 
            if (std::abs(t.x) < 10.0f && std::abs(t.z) < 10.0f) { t.x += 15.0f; } 
            ai.spawnX = t.x; ai.spawnZ = t.z; ai.isAlive = true; enemyList.push_back(e);
        }
    };

    auto resetToEditorState = [&]() {
        currentMode = EDIT_MODE; worldBlocks = backupWorldBlocks;
        Transform& marioTransform = ecs.getTransform(mario); PhysicsBody& marioPhysics = ecs.getPhysicsBody(mario);
        Transform& luigiTransform = ecs.getTransform(luigi); PhysicsBody& luigiPhysics = ecs.getPhysicsBody(luigi);
        marioTransform.x = backupPlayerX; marioTransform.y = backupPlayerY; marioTransform.z = backupPlayerZ;
        marioPhysics.velocityX = 0.0f; marioPhysics.velocityY = 0.0f; marioPhysics.velocityZ = 0.0f;
        luigiTransform.x = backupLuigiX; luigiTransform.y = backupLuigiY; luigiTransform.z = backupLuigiZ;
        luigiPhysics.velocityX = 0.0f; luigiPhysics.velocityZ = 0.0f; luigiPhysics.velocityY = 0.0f;
        playerHealth = playerMaxHealth;
        for (Entity e : enemyList) { ecs.getEnemyAI(e).isAlive = false; } enemyList.clear();
        for (const auto& sn : backupEnemyList) {
            Entity e = ecs.createEntity(); Transform& t = ecs.getTransform(e); EnemyAI& ai = ecs.getEnemyAI(e);
            t.x = sn.spawnX; t.z = sn.spawnZ; t.y = 0.5f; ai.isAlive = true; enemyList.push_back(e);
        }
        bulletList.clear(); playerGunLevel = 1; shootingCooldown = 0.0f; weaponMessageTimer = 0.0f;
        luigiHasGun = false; luigiShootingCooldown = 0.0f; luigiIsHelping = false; luigiMessageTimer = 0.0f;
    };

    auto enterSandboxEnvironment = [&]() {
        playerMaxHealth = 10; playerHealth = 10; damageCooldown = 0.0f; playerMovementSpeed = 15.0f; globalEnemySpeed = 4.0f;
        worldBlocks.clear(); eventQueue.push_back({ EVENT_CLEAR_BLOCKS }); eventQueue.push_back({ EVENT_PURGE_ENEMIES });
        Transform& marioTransform = ecs.getTransform(mario); PhysicsBody& marioPhysics = ecs.getPhysicsBody(mario);
        Transform& luigiTransform = ecs.getTransform(luigi); PhysicsBody& luigiPhysics = ecs.getPhysicsBody(luigi);
        marioTransform.x = 0.0f; marioTransform.y = 0.5f; marioTransform.z = 0.0f; marioTransform.rotationY = 0.0f;
        marioPhysics.velocityX = 0.0f; marioPhysics.velocityY = 0.0f; marioPhysics.velocityZ = 0.0f;
        luigiTransform.x = 2.0f; luigiTransform.y = 0.5f; luigiTransform.z = 0.0f; luigiTransform.rotationY = 0.0f;
        luigiPhysics.velocityX = 0.0f; luigiPhysics.velocityY = 0.0f; luigiPhysics.velocityZ = 0.0f;
        currentMode = EDIT_MODE; currentState = PLAYING; 
        bulletList.clear(); playerGunLevel = 1; isGunEnabled = false; weaponMessageTimer = 0.0f;
        luigiHasGun = false; luigiShootingCooldown = 0.0f; luigiIsHelping = false; luigiMessageTimer = 0.0f;
    };

    auto triggerLoadProcess = [&](int slot) {
        executeLoadOperation(slot, playerMovementSpeed, luigiMovementSpeed, globalEnemySpeed, playerMaxHealth, isGunEnabled, playerGunLevel, enemyList, ecs, worldBlocks);
        playerHealth = playerMaxHealth; backupWorldBlocks = worldBlocks;
        backupPlayerX = 0.0f; backupPlayerY = 0.5f; backupPlayerZ = 0.0f; backupLuigiX = 2.0f; backupLuigiY = 0.5f; backupLuigiZ = 0.0f;
        backupEnemyList.clear();
        for (Entity e : enemyList) { if (ecs.getEnemyAI(e).isAlive) backupEnemyList.push_back({ ecs.getTransform(e).x, ecs.getTransform(e).z }); }
        currentState = PLAYING; currentMode = EDIT_MODE;
    };

    float lastTime = myWindow.getTime(); glEnable(GL_DEPTH_TEST);

    while (!myWindow.shouldClose()) {
        float currentTime = myWindow.getTime(); float deltaTime = currentTime - lastTime; lastTime = currentTime;
        myWindow.pollEvents();

        Transform& marioTransform = ecs.getTransform(mario);
        PhysicsBody& marioPhysics = ecs.getPhysicsBody(mario);

        static bool escPressedLastFrame = false; bool escPressedThisFrame = myWindow.isKeyPressed(GLFW_KEY_ESCAPE);
        if (escPressedThisFrame && !escPressedLastFrame) { if (currentState == PLAYING) currentState = PAUSED; else if (currentState == PAUSED) currentState = PLAYING; }
        escPressedLastFrame = escPressedThisFrame;

        static bool ePressedLastFrame = false; bool ePressedThisFrame = myWindow.isKeyPressed(GLFW_KEY_E);
        if (ePressedThisFrame && !ePressedLastFrame) {
            if (currentMode == PLAY_MODE && currentState == PLAYING) {
                if (luigiIsHelping) {
                    luigiIsHelping = false; luigiMessageTimer = 5.0f; luigiMessage = "Agent is following";
                } else {
                    Transform& luigiTransform = ecs.getTransform(luigi);
                    float idx = marioTransform.x - luigiTransform.x; float idz = marioTransform.z - luigiTransform.z;
                    if (std::sqrt(idx*idx + idz*idz) < 3.0f) {
                        luigiIsHelping = true; luigiMessageTimer = 5.0f; luigiMessage = "Agent is helping";
                    }
                }
            }
        }
        ePressedLastFrame = ePressedThisFrame;

        ImGui_ImplOpenGL3_NewFrame(); ImGui_ImplGlfw_NewFrame(); ImGui::NewFrame();

        if (currentState == PLAYING || currentState == STAGE_COMPLETED) {
            spatialGrid.clear();
            if (enableSpatialGrid) {
                for (Entity e : enemyList) { if (ecs.getEnemyAI(e).isAlive) spatialGrid.insert(e, ecs.getTransform(e).x, ecs.getTransform(e).z); }
            }

            if (currentState == PLAYING) {
                if (weaponMessageTimer > 0.0f) weaponMessageTimer -= deltaTime;
                if (luigiMessageTimer > 0.0f) luigiMessageTimer -= deltaTime;

                marioPhysics.velocityX = 0.0f; marioPhysics.velocityZ = 0.0f; 
                if (myWindow.isKeyPressed(GLFW_KEY_D)) { marioPhysics.velocityX = playerMovementSpeed; marioTransform.rotationY = 90.0f; }
                if (myWindow.isKeyPressed(GLFW_KEY_A)) { marioPhysics.velocityX = -playerMovementSpeed; marioTransform.rotationY = -90.0f; }
                if (myWindow.isKeyPressed(GLFW_KEY_S)) { marioPhysics.velocityZ = playerMovementSpeed; marioTransform.rotationY = 0.0f; }
                if (myWindow.isKeyPressed(GLFW_KEY_W)) { marioPhysics.velocityZ = -playerMovementSpeed; marioTransform.rotationY = 180.0f; }
                
                marioTransform.x += marioPhysics.velocityX * deltaTime; marioTransform.z += marioPhysics.velocityZ * deltaTime;
                if (marioPhysics.velocityY > 0 && !myWindow.isKeyPressed(GLFW_KEY_SPACE)) marioPhysics.velocityY -= 50.0f * deltaTime; 
                else marioPhysics.velocityY -= 20.0f * deltaTime; 
                marioTransform.y += marioPhysics.velocityY * deltaTime;

                marioPhysics.isGrounded = false;
                if (marioTransform.y <= 0.5f) { marioTransform.y = 0.5f; marioPhysics.velocityY = 0.0f; marioPhysics.isGrounded = true; }

                // FIXED: Re-injected Mario's central 3D physics collision loop directly into his input execution node path
                if (currentMode == PLAY_MODE) {
                    for (int bIdx = 0; bIdx < worldBlocks.size(); ++bIdx) {
                        Resolve3DCollision(marioTransform.x, marioTransform.y, marioTransform.z, marioPhysics.velocityY, marioPhysics.isGrounded, worldBlocks[bIdx], 0.35f, 0.45f, 0.35f, bIdx, eventQueue, currentMode, 1);
                        if (worldBlocks[bIdx].type == ASSET_TRAP && std::abs(marioTransform.x - worldBlocks[bIdx].x) < 1.4f && std::abs(marioTransform.z - worldBlocks[bIdx].z) < 1.4f) {
                            if (damageCooldown <= 0.0f && !isPlayerInvulnerable) eventQueue.push_back({ EVENT_PLAYER_DAMAGE });
                        }
                        if (worldBlocks[bIdx].type == ASSET_GOAL && std::abs(marioTransform.x - worldBlocks[bIdx].x) < 1.2f && std::abs(marioTransform.z - worldBlocks[bIdx].z) < 1.2f) {
                            currentState = STAGE_COMPLETED; 
                        }
                    }
                    
                    // Update behaviors for automated entities (enemies and Luigi)
                    UpdateBehaviorSystem(ecs, enemyList, worldBlocks, mario, luigi, deltaTime, globalEnemySpeed, luigiMovementSpeed, luigiIsHelping, enableSpatialGrid, spatialGrid, enableEnemyJumping, isPlayerInvulnerable, playerHealth, damageCooldown, eventQueue, currentMode, luigiHasGun, luigiShootingCooldown, bulletList);

                    if (marioTransform.x < -99.5f) marioTransform.x = -99.5f; if (marioTransform.z < -99.5f) marioTransform.z = -99.5f;
                    if (marioTransform.y < -6.0f) { resetToEditorState(); }
                    if (myWindow.isKeyPressed(GLFW_KEY_SPACE) && marioPhysics.isGrounded) { marioPhysics.velocityY = 12.0f; marioPhysics.isGrounded = false; }
                } 
                // FIXED: Re-injected structural Edit Mode coordinate alignment locks for both player and ally entities
                else if (currentMode == EDIT_MODE) {
                    for (const auto& block : worldBlocks) {
                        Resolve3DCollision(marioTransform.x, marioTransform.y, marioTransform.z, marioPhysics.velocityY, marioPhysics.isGrounded, block, 0.35f, 0.45f, 0.35f, 0, eventQueue, currentMode, 1);
                    }
                    PhysicsBody& luigiPhysics = ecs.getPhysicsBody(luigi);
                    Transform& luigiTransform = ecs.getTransform(luigi);
                    luigiPhysics.velocityY = 0.0f; luigiTransform.y = 0.5f;
                    luigiTransform.x = marioTransform.x + 2.0f; luigiTransform.z = marioTransform.z;
                }

                // --- MASTER GUN EXECUTION NODE ---
                if (isGunEnabled && currentMode == PLAY_MODE) {
                    if (shootingCooldown > 0.0f) shootingCooldown -= deltaTime;
                    if (glfwGetMouseButton(myWindow.getRawWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && shootingCooldown <= 0.0f) {
                        for (int i = 0; i < playerGunLevel; i++) {
                            Projectile p; p.x = marioTransform.x; p.y = marioTransform.y + 0.1f; p.z = marioTransform.z;
                            p.speed = 35.0f; p.lifetime = 1.5f; p.isAlive = true;
                            float offsetAngleDeg = (i - (playerGunLevel - 1) / 2.0f) * 8.0f; 
                            float finalRad = (marioTransform.rotationY + offsetAngleDeg) * (3.14159265f / 180.0f);
                            p.dirX = sin(finalRad); p.dirZ = cos(finalRad); bulletList.push_back(p);
                        }
                        shootingCooldown = 0.22f; 
                    }
                }
                
                for (auto& p : bulletList) {
                    if (!p.isAlive) continue;
                    p.x += p.dirX * p.speed * deltaTime; p.z += p.dirZ * p.speed * deltaTime;
                    p.lifetime -= deltaTime; if (p.lifetime <= 0.0f) p.isAlive = false;

                    std::vector<Entity> nearbyEnemies = enableSpatialGrid ? spatialGrid.getNearbyEntities(p.x, p.z, 2.0f) : enemyList;
                    for (Entity otherE : nearbyEnemies) {
                        EnemyAI& ai = ecs.getEnemyAI(otherE); if (!ai.isAlive) continue;
                        Transform& et = ecs.getTransform(otherE);
                        float dx = et.x - p.x; float dz = et.z - p.z; float distSq = dx*dx + dz*dz;
                        if (distSq < 0.9f && std::abs(et.y - p.y) < 1.0f) { 
                            ai.isAlive = false; p.isAlive = false; 
                            if (enableSpatialGrid) spatialGrid.insert(otherE, 999.0f, 999.0f); 
                            break; 
                        }
                    }
                }
                bulletList.erase(std::remove_if(bulletList.begin(), bulletList.end(), [](const Projectile& p){ return !p.isAlive; }), bulletList.end());
            }
            if (playerHealth <= 0) { resetToEditorState(); }
        }

        // --- IMGUI ENVIRONMENTAL MAIN INTERFACE CONTROLS ---
        if (currentState == MAIN_MENU) {
            ImGui::SetNextWindowPos(ImVec2(1440.0f / 2.0f, 900.0f / 2.0f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::Begin("Main Menu Container", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);
            if (currentMenuSubState == SUB_MAIN) {
                ImGui::SetNextWindowPos(ImVec2(1440.0f / 2.0f, 900.0f / 2.0f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
                ImGui::SetWindowFontScale(2.5f); ImGui::Text("VIRTUAL ENVIRONMENT MAKER"); ImGui::SetWindowFontScale(1.5f);
                ImGui::Dummy(ImVec2(0.0f, 20.0f)); 
                if (ImGui::Button("New Environment", ImVec2(450, 60))) { enterSandboxEnvironment(); } ImGui::Dummy(ImVec2(0.0f, 5.0f)); 
                if (ImGui::Button("Load Environment", ImVec2(450, 60))) { currentMenuSubState = SUB_LOAD_SELECT; currentSaveSubState = SAVE_INACTIVE; } ImGui::Dummy(ImVec2(0.0f, 5.0f)); 
                if (ImGui::Button("Quit to Desktop", ImVec2(450, 60))) { glfwSetWindowShouldClose(myWindow.getRawWindow(), true); }
            } 
            else if (currentMenuSubState == SUB_LOAD_SELECT) {
                if (currentSaveSubState == SAVE_DELETE_CONFIRM) {
                    ImGui::SetWindowFontScale(2.0f); ImGui::Text("Confirm Layout File Deletion"); ImGui::SetWindowFontScale(1.2f);
                    ImGui::Dummy(ImVec2(0.0f, 15.0f));
                    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Environment %d configuration will be permanently unlinked from disk.\nProceed?", pendingActiveTargetSlot);
                    ImGui::Dummy(ImVec2(0.0f, 20.0f));
                    if (ImGui::Button("Yes, Delete File", ImVec2(210, 45))) {
                        std::string pathName = "slot" + std::to_string(pendingActiveTargetSlot) + ".env";
                        std::remove(pathName.c_str()); currentSaveSubState = SAVE_INACTIVE;
                    } ImGui::SameLine();
                    if (ImGui::Button("No, Cancel", ImVec2(210, 45))) { currentSaveSubState = SAVE_INACTIVE; }
                }
                else {
                    ImGui::SetWindowFontScale(2.0f); ImGui::Text("Select Environment Configuration to Load"); ImGui::SetWindowFontScale(1.2f);
                    ImGui::Dummy(ImVec2(0.0f, 15.0f));
                    for (int slot = 1; slot <= 3; ++slot) {
                        std::string label = "Slot " + std::to_string(slot) + ": Environment " + std::to_string(slot);
                        bool exists = CheckFileExists("slot" + std::to_string(slot) + ".env");
                        label += exists ? " (saved)" : " (empty)";
                        if (ImGui::Button(label.c_str(), ImVec2(340, 50))) { if (exists) { triggerLoadProcess(slot); } }
                        if (exists) {
                            ImGui::SameLine();
                            if (ImGui::Button(("Delete##" + std::to_string(slot)).c_str(), ImVec2(100, 50))) {
                                pendingActiveTargetSlot = slot; currentSaveSubState = SAVE_DELETE_CONFIRM;
                            }
                        }
                        ImGui::Dummy(ImVec2(0.0f, 5.0f));
                    }
                    ImGui::Dummy(ImVec2(0.0f, 15.0f));
                    if (ImGui::Button("<- Return to Main Menu", ImVec2(240, 40))) { currentMenuSubState = SUB_MAIN; }
                }
            }
            ImGui::End();
        }

        if (currentState == PAUSED) {
            ImGui::SetNextWindowPos(ImVec2(1440.0f / 2.0f, 900.0f / 2.0f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::Begin("Pause Menu", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);
            ImGui::SetWindowFontScale(2.5f); ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "ENVIRONMENT PAUSED"); ImGui::SetWindowFontScale(1.5f);
            ImGui::Dummy(ImVec2(0.0f, 20.0f));
            if (ImGui::Button("Resume Creator", ImVec2(300, 50))) { currentState = PLAYING; } ImGui::Dummy(ImVec2(0.0f, 5.0f)); 
            if (ImGui::Button("Return to Main Menu", ImVec2(300, 50))) { currentState = MAIN_MENU; currentMenuSubState = SUB_MAIN; }
            ImGui::End();
        }

        if (currentState == STAGE_COMPLETED) {
            ImGui::SetNextWindowPos(ImVec2(1440.0f / 2.0f, 900.0f / 2.0f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::Begin("Victory Context Frame", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);
            ImGui::SetWindowFontScale(2.5f); ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "VICTORY! STAGE COMPLETED"); ImGui::SetWindowFontScale(1.2f);
            ImGui::Dummy(ImVec2(0.0f, 15.0f));
            if (ImGui::Button("Return to Environment Editor", ImVec2(-1, 50))) { currentState = PLAYING; resetToEditorState(); }
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
                if (ImGui::Button("Stop Playtest and Reset Layout", ImVec2(320, 45))) { resetToEditorState(); }
                ImGui::End();
            }

            if (currentMode == EDIT_MODE) {
                ImGui::SetNextWindowPos(ImVec2(20.0f, 80.0f), ImGuiCond_Once); ImGui::SetNextWindowSize(ImVec2(480.0f, 780.0f), ImGuiCond_Once);
                ImGui::Begin("Environment Control Panel"); ImGui::SetWindowFontScale(1.15f);
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Execution Controller Matrix"); ImGui::Separator();
                if (ImGui::Button("Start Playtest Simulation", ImVec2(-1, 45))) {
                    currentMode = PLAY_MODE; backupWorldBlocks = worldBlocks;
                    backupPlayerX = marioTransform.x; backupPlayerY = marioTransform.y; backupPlayerZ = marioTransform.z;
                    Transform& luigiTransform = ecs.getTransform(luigi); backupLuigiX = luigiTransform.x; backupLuigiY = luigiTransform.y; backupLuigiZ = luigiTransform.z;
                    backupEnemyList.clear();
                    for (Entity e : enemyList) { if (ecs.getEnemyAI(e).isAlive) backupEnemyList.push_back({ ecs.getTransform(e).x, ecs.getTransform(e).z }); }
                }
                ImGui::Dummy(ImVec2(0, 5));

                if (ImGui::Button("Export Environment Configuration File", ImVec2(-1, 35))) { currentSaveSubState = SAVE_SLOT_SELECT; }
                if (currentSaveSubState == SAVE_SLOT_SELECT) {
                    ImGui::Indent(); ImGui::TextColored(ImVec4(1, 1, 0, 1), "Select Destination Profile Slot:");
                    for (int slot = 1; slot <= 3; ++slot) {
                        std::string sLabel = "Slot " + std::to_string(slot); sLabel += CheckFileExists("slot" + std::to_string(slot) + ".env") ? " (saved)" : " (empty)";
                        if (ImGui::Button(sLabel.c_str(), ImVec2(-1, 28))) {
                            pendingActiveTargetSlot = slot;
                            if (CheckFileExists("slot" + std::to_string(slot) + ".env")) { currentSaveSubState = SAVE_OVERWRITE_CONFIRM; } 
                            else { executeSaveOperation(slot, playerMovementSpeed, luigiMovementSpeed, globalEnemySpeed, playerMaxHealth, isGunEnabled, playerGunLevel, enemyList, ecs, worldBlocks); currentSaveSubState = SAVE_INACTIVE; }
                        }
                    }
                    if (ImGui::Button("Cancel Export", ImVec2(-1, 25))) { currentSaveSubState = SAVE_INACTIVE; }
                    ImGui::Unindent(); ImGui::Separator();
                }
                else if (currentSaveSubState == SAVE_OVERWRITE_CONFIRM) {
                    ImGui::Indent(); ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "Environment %d contains saved configs.\nProceed to overwrite?", pendingActiveTargetSlot);
                    if (ImGui::Button("Yes, Overwrite", ImVec2(180, 30))) {
                        executeSaveOperation(pendingActiveTargetSlot, playerMovementSpeed, luigiMovementSpeed, globalEnemySpeed, playerMaxHealth, isGunEnabled, playerGunLevel, enemyList, ecs, worldBlocks); currentSaveSubState = SAVE_INACTIVE;
                    } ImGui::SameLine();
                    if (ImGui::Button("No, Cancel", ImVec2(180, 30))) { currentSaveSubState = SAVE_INACTIVE; }
                    ImGui::Unindent(); ImGui::Separator();
                }
                ImGui::Dummy(ImVec2(0, 10));

                float padding = ImGui::GetStyle().ItemSpacing.x; float panelWidth = ImGui::GetContentRegionAvail().x;
                float threeColumnWidth = (panelWidth - (2.0f * padding)) / 3.0f; float twoColumnWidth = (panelWidth - padding) / 2.0f;

                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Simulation Target Spawning"); ImGui::Separator();
                if (ImGui::Button("Swarm x1", ImVec2(threeColumnWidth, 32))) { eventQueue.push_back({ EVENT_SPAWN_ENEMY, ASSET_ROCK, 0, 0, 0, 1 }); } ImGui::SameLine();
                if (ImGui::Button("Swarm x10", ImVec2(threeColumnWidth, 32))) { eventQueue.push_back({ EVENT_SPAWN_ENEMY, ASSET_ROCK, 0, 0, 0, 10 }); } ImGui::SameLine();
                if (ImGui::Button("Overload x1000", ImVec2(threeColumnWidth, 32))) { eventQueue.push_back({ EVENT_SPAWN_ENEMY, ASSET_ROCK, 0, 0, 0, 1000 }); }
                
                float rad = marioTransform.rotationY * (3.14159265f / 180.0f); float pX = marioTransform.x + sin(rad) * 4.0f; float pZ = marioTransform.z + cos(rad) * 4.0f;

                ImGui::Dummy(ImVec2(0, 5)); ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Visual Palette Generation Toolbar"); ImGui::Separator();
                if (ImGui::Button("Item Block", ImVec2(threeColumnWidth, 40))) { eventQueue.push_back({ EVENT_SPAWN_ASSET, ASSET_BLOCK, pX, 1.5f, pZ, 0 }); } ImGui::SameLine();
                if (ImGui::Button("Rock Block", ImVec2(threeColumnWidth, 40))) { eventQueue.push_back({ EVENT_SPAWN_ASSET, ASSET_ROCK, pX, 0.5f, pZ, 0 }); } ImGui::SameLine();
                if (ImGui::Button("Small Hill", ImVec2(threeColumnWidth, 40))) { eventQueue.push_back({ EVENT_SPAWN_ASSET, ASSET_HILL, pX, 0.2f, pZ, 0 }); }
                
                if (ImGui::Button("Wood Tree", ImVec2(threeColumnWidth, 40))) { eventQueue.push_back({ EVENT_SPAWN_ASSET, ASSET_TREE, pX, 0.0f, pZ, 0 }); } ImGui::SameLine();
                if (ImGui::Button("Hazard Vine", ImVec2(threeColumnWidth, 40))) { eventQueue.push_back({ EVENT_SPAWN_ASSET, ASSET_TRAP, pX, 0.0f, pZ, 0 }); } ImGui::SameLine();
                if (ImGui::Button("Goal Pole", ImVec2(threeColumnWidth, 40))) { eventQueue.push_back({ EVENT_SPAWN_ASSET, ASSET_GOAL, pX, 0.0f, pZ, 0 }); }
                
                if (ImGui::Button("Clear Placed Objects", ImVec2(twoColumnWidth, 30))) { eventQueue.push_back({ EVENT_CLEAR_BLOCKS }); } ImGui::SameLine();
                if (ImGui::Button("Purge All Enemies", ImVec2(twoColumnWidth, 30))) { eventQueue.push_back({ EVENT_PURGE_ENEMIES }); }

                ImGui::Dummy(ImVec2(0, 10)); ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Player Health Sandbox Matrices"); ImGui::Separator();
                ImGui::Checkbox("Player Invulnerability Toggle", &isPlayerInvulnerable);
                ImGui::PushItemWidth(-170.0f);
                if(ImGui::SliderInt("Max Health Capacity", &playerMaxHealth, 1, 10)) { playerHealth = playerMaxHealth; }
                ImGui::SliderInt("Current Player Health", &playerHealth, 0, playerMaxHealth); ImGui::PopItemWidth();

                ImGui::Dummy(ImVec2(0, 10)); ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Player Weapon Matrix"); ImGui::Separator();
                ImGui::Checkbox("Sandbox: Enable Player Gun", &isGunEnabled); ImGui::SliderInt("Active Barrels Level", &playerGunLevel, 1, 5);

                ImGui::Dummy(ImVec2(0, 10)); ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Global Simulation Sliders"); ImGui::Separator();
                ImGui::PushItemWidth(-170.0f); ImGui::SliderFloat("Player Movement Speed", &playerMovementSpeed, 5.0f, 40.0f, "%.1f units/s");
                ImGui::SliderFloat("Luigi Movement Speed", &luigiMovementSpeed, 5.0f, 30.0f, "%.1f units/s");
                ImGui::SliderFloat("Enemy Tracking Speed", &globalEnemySpeed, 0.0f, 25.0f, "%.1f units/s"); ImGui::PopItemWidth();
                ImGui::Checkbox("Enable Enemy Jumping Force", &enableEnemyJumping);

                ImGui::Dummy(ImVec2(0, 10)); ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Material Vector Color Field"); ImGui::Separator();
                ImGui::PushItemWidth(-170.0f); ImGui::ColorEdit3("Swarm Enemy Model", enemyColor); ImGui::ColorEdit3("Player Avatar Model", characterColor); ImGui::PopItemWidth();

                ImGui::Dummy(ImVec2(0, 10)); ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Optimization Toggles"); ImGui::Separator();
                ImGui::Checkbox("Enable Spatial Hash Partitioning Grid", &enableSpatialGrid);
                int activeEnemies = 0; for(Entity e : enemyList) { if(ecs.getEnemyAI(e).isAlive) activeEnemies++; }
                ImGui::Dummy(ImVec2(0, 10)); ImGui::Separator(); ImGui::Text("Active Enemies: %d / 3000 Max | Assets Placed: %lu", activeEnemies, worldBlocks.size()); ImGui::End();
            }

            ImGui::SetNextWindowPos(ImVec2(1420.0f, 850.0f), ImGuiCond_Always, ImVec2(1.0f, 1.0f));
            ImGui::Begin("InstructionsOverlay", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs);
            ImGui::SetWindowFontScale(1.3f);
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.6f), "Drive character in EDIT mode with WASD to build across the map canvas");
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.6f), "Press [E] when near Luigi to activate attack assistance routines");
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.6f), "Press [ESC] to view configuration system parameters"); ImGui::End();

            ImGui::SetNextWindowPos(ImVec2(1420.0f, 20.0f), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
            ImGui::Begin("FPSOverlay", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs);
            ImGui::SetWindowFontScale(1.5f); ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "FPS: %.1f", io.Framerate);
            float frameTimeMs = 1000.0f / io.Framerate; ImVec4 budgetColor = (frameTimeMs > 16.67f) ? ImVec4(1.0f, 0.4f, 0.4f, 1.0f) : ImVec4(0.4f, 1.0f, 0.4f, 1.0f);
            ImGui::TextColored(budgetColor, "Frame: %.2f ms", frameTimeMs); ImGui::End();

            if (weaponMessageTimer > 0.0f && currentMode == PLAY_MODE) {
                ImGui::SetNextWindowPos(ImVec2(1440.0f / 2.0f, 900.0f - 120.0f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
                ImGui::Begin("WeaponAlertOverlay", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs);
                ImGui::SetWindowFontScale(1.8f); ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Max weapon amount reached"); ImGui::End();
            }

            if (luigiMessageTimer > 0.0f && currentMode == PLAY_MODE) {
                ImGui::SetNextWindowPos(ImVec2(1440.0f / 2.0f, 900.0f - 160.0f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
                ImGui::Begin("LuigiAlertOverlay", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs);
                ImGui::SetWindowFontScale(1.8f); ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%s", luigiMessage.c_str()); ImGui::End();
            }
        }

        // --- ASSET EVENT DISPATCHER MODULAR CALL ---
        ProcessEventSystem(eventQueue, worldBlocks, enemyList, ecs, playerMaxHealth, playerHealth, damageCooldown, playerGunLevel, weaponMessageTimer, luigiHasGun, handleSpawnEnemy);
        eventQueue.clear(); 

        // --- RENDER UPDATE ---
        glClearColor(0.12f, 0.28f, 0.44f, 1.0f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if (currentState != MAIN_MENU) {
            glMatrixMode(GL_PROJECTION); glLoadIdentity(); glFrustum(-1.6, 1.6, -1.0, 1.0, 2.0, 200.0); 
            glMatrixMode(GL_MODELVIEW); glLoadIdentity(); glRotatef(25.0f, 1.0f, 0.0f, 0.0f); glTranslatef(-marioTransform.x, -8.0f, -18.0f - marioTransform.z); 

            glPushMatrix(); glTranslatef(marioTransform.x, 0.01f, marioTransform.z); DrawShadow(0.6f); glPopMatrix();
            Transform& luigiTransform = ecs.getTransform(luigi); glPushMatrix(); glTranslatef(luigiTransform.x, 0.01f, luigiTransform.z); DrawShadow(0.6f); glPopMatrix();
            for (Entity e : enemyList) { if (ecs.getEnemyAI(e).isAlive) { glPushMatrix(); glTranslatef(ecs.getTransform(e).x, 0.01f, ecs.getTransform(e).z); DrawShadow(0.7f); glPopMatrix(); } }

            glEnable(GL_LIGHTING); DrawPlatform(); 
            for (const auto& block : worldBlocks) { DrawCustomAsset(block); }
            for (Entity e : enemyList) { if (ecs.getEnemyAI(e).isAlive) { glPushMatrix(); glTranslatef(ecs.getTransform(e).x, ecs.getTransform(e).y, ecs.getTransform(e).z); DrawEnemy(enemyColor[0], enemyColor[1], enemyColor[2]); glPopMatrix(); } }

            glPushMatrix(); glTranslatef(marioTransform.x, marioTransform.y, marioTransform.z); glRotatef(marioTransform.rotationY, 0.0f, 1.0f, 0.0f); 
            glPushMatrix(); glTranslatef(0.0f, 0.4f, 0.5f); glScalef(0.1f, 0.1f, 0.1f); DrawProceduralCube(1.0f, 0.0f, 0.0f, 0.0f); glPopMatrix(); 
            DrawPlayer(characterColor[0], characterColor[1], characterColor[2], playerGunLevel, isGunEnabled); glPopMatrix();

            glPushMatrix(); glTranslatef(luigiTransform.x, luigiTransform.y, luigiTransform.z); glRotatef(luigiTransform.rotationY, 0.0f, 1.0f, 0.0f);
            glPushMatrix(); glTranslatef(0.0f, 0.4f, 0.5f); glScalef(0.1f, 0.1f, 0.1f); DrawProceduralCube(0.0f, 1.0f, 0.0f, 0.0f); glPopMatrix();
            DrawPlayer(0.12f, 0.58f, 0.16f, luigiHasGun ? 1 : 0, isGunEnabled); glPopMatrix();
            
            glPushMatrix(); 
            for (const auto& p : bulletList) {
                if (p.isAlive) { glPushMatrix(); glTranslatef(p.x, p.y, p.z); glScalef(0.15f, 0.15f, 0.15f); DrawProceduralCube(1.0f, 0.8f, 0.0f, 0.0f); glPopMatrix(); }
            }
            glPopMatrix(); glEnable(GL_LIGHTING);

            glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity(); glOrtho(0.0, 1440.0, 900.0, 0.0, -1.0, 1.0); 
            glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity(); glDisable(GL_DEPTH_TEST); 
            float barWidth = 400.0f; float barHeight = 30.0f; float startX = (1440.0f / 2.0f) - (barWidth / 2.0f); float startY = 900.0f - barHeight - 20.0f; 
            glBegin(GL_QUADS); glColor3f(0.3f, 0.0f, 0.0f); glVertex2f(startX, startY); glVertex2f(startX + barWidth, startY); glVertex2f(startX + barWidth, startY + barHeight); glVertex2f(startX, startY + barHeight); glEnd();
            glBegin(GL_QUADS); glColor3f(0.0f, 1.0f, 0.0f);
            float healthRatio = (playerMaxHealth > 0) ? ((float)playerHealth / (float)playerMaxHealth) : 0.0f; float healthWidth = barWidth * healthRatio;
            if (healthWidth > 0.0f) { glVertex2f(startX, startY); glVertex2f(startX + healthWidth, startY); glVertex2f(startX + healthWidth, startY + barHeight); glVertex2f(startX, barHeight + startY); }
            glEnd(); glEnable(GL_DEPTH_TEST); glMatrixMode(GL_MODELVIEW); glPopMatrix(); glMatrixMode(GL_PROJECTION); glPopMatrix();
        }
        ImGui::Render(); ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); myWindow.swapBuffers();
    }
    ImGui_ImplOpenGL3_Shutdown(); ImGui_ImplGlfw_Shutdown(); ImGui::DestroyContext(); return 0;
}