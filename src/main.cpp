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

enum GameState { MAIN_MENU, COUNTDOWN, PLAYING, PAUSED, FLAG_ANIMATION, STAGE_COMPLETED, GAME_OVER };
enum GameMode { CLASSIC, PRACTICE };

// --- 3D DRAWING HELPERS ---
void DrawUnitCube(float r, float g, float b) {
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f); glColor3f(r, g, b); glVertex3f(-0.5f, -0.5f,  0.5f); glVertex3f( 0.5f, -0.5f,  0.5f); glVertex3f( 0.5f,  0.5f,  0.5f); glVertex3f(-0.5f,  0.5f,  0.5f);
    glNormal3f(0.0f, 0.0f, -1.0f); glColor3f(r*0.8f, g*0.8f, b*0.8f); glVertex3f( 0.5f, -0.5f, -0.5f); glVertex3f( 0.5f,  0.5f, -0.5f); glVertex3f( 0.5f,  0.5f,  0.5f); glVertex3f( 0.5f, -0.5f,  0.5f);
    glNormal3f(0.0f, 1.0f, 0.0f); glColor3f(r*0.9f, g*0.9f, b*0.9f); glVertex3f(-0.5f,  0.5f, -0.5f); glVertex3f(-0.5f,  0.5f,  0.5f); glVertex3f( 0.5f,  0.5f,  0.5f); glVertex3f( 0.5f,  0.5f, -0.5f);
    glNormal3f(0.0f, -1.0f, 0.0f); glColor3f(r*0.7f, g*0.7f, b*0.7f); glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f(-0.5f, -0.5f,  0.5f); glVertex3f(-0.5f,  0.5f,  0.5f); glVertex3f(-0.5f,  0.5f, -0.5f);
    glNormal3f(1.0f, 0.0f, 0.0f); glColor3f(r*0.6f, g*0.6f, b*0.6f); glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f( 0.5f, -0.5f, -0.5f); glVertex3f( 0.5f,  0.5f, -0.5f); glVertex3f(-0.5f,  0.5f, -0.5f);
    glNormal3f(-1.0f, 0.0f, 0.0f); glColor3f(r*0.5f, g*0.5f, b*0.5f); glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f( 0.5f, -0.5f, -0.5f); glVertex3f( 0.5f, -0.5f,  0.5f); glVertex3f(-0.5f, -0.5f,  0.5f);
    glEnd();
}

void DrawPlayer() {
    glPushMatrix(); glTranslatef(0.0f, 0.1f, 0.0f); glScalef(0.7f, 0.9f, 0.7f); DrawUnitCube(0.9f, 0.1f, 0.1f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, 0.25f, 0.35f); glScalef(0.5f, 0.3f, 0.1f); DrawUnitCube(0.9f, 0.9f, 1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.15f, -0.35f, 0.0f); glScalef(0.25f, 0.3f, 0.25f); DrawUnitCube(0.9f, 0.1f, 0.1f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.15f, -0.35f, 0.0f); glScalef(0.25f, 0.3f, 0.25f); DrawUnitCube(0.9f, 0.1f, 0.1f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, 0.55f, 0.0f); glScalef(0.75f, 0.05f, 0.75f); DrawUnitCube(1.0f, 0.0f, 0.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, 0.6f, 0.0f); glScalef(0.6f, 0.1f, 0.6f); DrawUnitCube(1.0f, 1.0f, 1.0f); glPopMatrix();
}

void DrawEnemy() {
    glPushMatrix(); glTranslatef(0.0f, 0.0f, 0.0f); glScalef(0.8f, 0.6f, 0.8f); DrawUnitCube(0.4f, 0.2f, 0.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, 0.4f, 0.0f); glScalef(0.9f, 0.3f, 0.9f); DrawUnitCube(0.6f, 0.3f, 0.1f); glPopMatrix();
}

void DrawPlatform() {
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glColor3f(0.0f, 0.6f, 0.2f); glVertex3f(-100.0f, 0.0f,  100.0f); glVertex3f( 100.0f, 0.0f,  100.0f); glVertex3f( 100.0f, 0.0f, -100.0f); glVertex3f(-100.0f, 0.0f, -100.0f);
    glNormal3f(0.0f, -1.0f, 0.0f);
    glColor3f(0.3f, 0.2f, 0.1f); glVertex3f(-100.0f, -5.0f,  100.0f); glVertex3f( 100.0f, -5.0f,  100.0f); glVertex3f( 100.0f, -5.0f, -100.0f); glVertex3f(-100.0f, -5.0f, -100.0f);
    glEnd();
}

// NEW: Blob Shadow Function
void DrawShadow(float radius) {
    glDisable(GL_LIGHTING); // Shadows don't need lighting
    glEnable(GL_BLEND); // Turn on transparency
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE); // Prevent visual glitching with the floor

    glColor4f(0.0f, 0.0f, 0.0f, 0.4f); // 40% transparent black
    glBegin(GL_QUADS);
    glVertex3f(-radius, 0.0f,  radius);
    glVertex3f( radius, 0.0f,  radius);
    glVertex3f( radius, 0.0f, -radius);
    glVertex3f(-radius, 0.0f, -radius);
    glEnd();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

void DrawFlagpole(float currentFlagHeight) {
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glColor3f(0.8f, 0.8f, 0.0f); glVertex3f(-0.1f, 0.0f, 0.1f); glVertex3f( 0.1f, 0.0f, 0.1f); glVertex3f( 0.1f, 5.0f, 0.1f); glVertex3f(-0.1f, 5.0f, 0.1f);
    glColor3f(1.0f, 1.0f, 1.0f); glVertex3f( 0.1f, currentFlagHeight, 0.0f); glVertex3f( 1.5f, currentFlagHeight, 0.0f); glVertex3f( 1.5f, currentFlagHeight + 1.0f, 0.0f); glVertex3f( 0.1f, currentFlagHeight + 1.0f, 0.0f);
    glEnd();
}

void DrawRect2D(float x, float y, float width, float height) {
    glBegin(GL_QUADS); glVertex2f(x, y); glVertex2f(x + width, y); glVertex2f(x + width, y + height); glVertex2f(x, y + height); glEnd();
}

void DrawRetroText(int symbol, float x, float y) {
    glColor3f(1.0f, 0.8f, 0.0f); 
    float s = 25.0f; float w = 120.0f; float h = 200.0f; 
    if (symbol == 3) { DrawRect2D(x, y, w, s); DrawRect2D(x, y + h/2 - s/2, w, s); DrawRect2D(x, y + h - s, w, s); DrawRect2D(x + w - s, y, s, h); }
    else if (symbol == 2) { DrawRect2D(x, y, w, s); DrawRect2D(x + w - s, y, s, h/2); DrawRect2D(x, y + h/2 - s/2, w, s); DrawRect2D(x, y + h/2, s, h/2); DrawRect2D(x, y + h - s, w, s); }
    else if (symbol == 1) { DrawRect2D(x + w/2 - s/2, y, s, h); }
    else if (symbol == 0) { 
        DrawRect2D(x-80.0f, y, w, s); DrawRect2D(x-80.0f, y, s, h); DrawRect2D(x-80.0f, y + h - s, w, s); DrawRect2D(x-80.0f + w - s, y + h/2, s, h/2); DrawRect2D(x-80.0f + w/2, y + h/2, w/2, s); 
        DrawRect2D(x+80.0f, y, w, s); DrawRect2D(x+80.0f, y, s, h); DrawRect2D(x+80.0f + w - s, y, s, h); DrawRect2D(x+80.0f, y + h - s, w, s); 
    }
}

int main() {
    srand(static_cast<unsigned int>(time(0))); 
    Window myWindow(1440, 900, "VIE-Project Engine");
    Registry ecs;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(myWindow.getRawWindow(), true); 
    ImGui_ImplOpenGL3_Init("#version 120");

    SpatialGrid spatialGrid(4.0f);

    float globalEnemySpeed = 4.0f;
    bool enableSpatialGrid = true; 
    bool enableEnemyJumping = false; // NEW: The jumping toggle
    bool showDebugUI = false; 
    
    std::string inGameMessage = "";
    float messageTimer = 0.0f;

    Entity mario = ecs.createEntity();
    std::vector<Entity> enemyList;
    
    GameState currentState = MAIN_MENU; 
    GameMode currentMode = PRACTICE;
    int currentWave = 1;
    
    float flagX = 28.0f; float flagZ = -28.0f; float flagHeight = 4.0f; 
    int playerHealth = 10; 
    int currentScore = 0;
    float damageCooldown = 0.0f; 
    float countdownTimer = 3.99f;
    int currentSecond = 4; 

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL); 
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    GLfloat light_position[] = { 10.0f, 50.0f, 20.0f, 1.0f }; 
    GLfloat light_ambient[] = { 0.4f, 0.4f, 0.4f, 1.0f }; 
    GLfloat light_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f }; 
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);

    auto spawnEnemies = [&](int count) {
        for (int i = 0; i < count; i++) {
            Entity e = ecs.createEntity();
            Transform& t = ecs.getTransform(e);
            EnemyAI& ai = ecs.getEnemyAI(e);
            t.x = (rand() % 80) - 40.0f; t.z = (rand() % 80) - 40.0f; t.y = 0.5f; 
            
            if (std::abs(t.x) < 10.0f && std::abs(t.z) < 10.0f) { t.x += 15.0f; } 
            
            ai.spawnX = t.x; ai.spawnZ = t.z; ai.isAlive = true;
            enemyList.push_back(e);
        }
    };

    auto resetGame = [&](GameMode selectedMode) {
        currentMode = selectedMode;
        playerHealth = 10;
        currentScore = 0;
        currentWave = 1;
        damageCooldown = 0.0f;
        countdownTimer = 3.99f;
        currentSecond = 4;
        flagHeight = 4.0f;
        inGameMessage = "";
        
        Transform& mt = ecs.getTransform(mario);
        mt.x = 0.0f; mt.y = 8.0f; mt.z = 0.0f; mt.rotationY = 0.0f;
        PhysicsBody& mp = ecs.getPhysicsBody(mario);
        mp.velocityX = 0.0f; mp.velocityY = 0.0f; mp.velocityZ = 0.0f;

        for (Entity e : enemyList) { ecs.getEnemyAI(e).isAlive = false; }
        enemyList.clear();

        if (currentMode == CLASSIC) {
            spawnEnemies(10); 
            showDebugUI = false; 
            currentState = COUNTDOWN; 
        } else {
            spawnEnemies(0); 
            showDebugUI = true; 
            currentState = PLAYING; 
        }
    };

    float lastTime = myWindow.getTime();
    glEnable(GL_DEPTH_TEST);

    while (!myWindow.shouldClose()) {
        float currentTime = myWindow.getTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        myWindow.pollEvents();
        
        static bool escPressedLastFrame = false;
        bool escPressedThisFrame = myWindow.isKeyPressed(GLFW_KEY_ESCAPE);
        if (escPressedThisFrame && !escPressedLastFrame) {
            if (currentState == PLAYING) currentState = PAUSED;
            else if (currentState == PAUSED) currentState = PLAYING;
        }
        escPressedLastFrame = escPressedThisFrame;

        static bool tabPressedLastFrame = false;
        bool tabPressedThisFrame = myWindow.isKeyPressed(GLFW_KEY_TAB);
        if (tabPressedThisFrame && !tabPressedLastFrame && currentState == PLAYING && currentMode == PRACTICE) { 
            showDebugUI = !showDebugUI; 
        }
        tabPressedLastFrame = tabPressedThisFrame;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        Transform& marioTransform = ecs.getTransform(mario);
        PhysicsBody& marioPhysics = ecs.getPhysicsBody(mario);

        if (currentState != PAUSED && currentState != MAIN_MENU && currentState != GAME_OVER && currentState != STAGE_COMPLETED) {
            if (damageCooldown > 0.0f) damageCooldown -= deltaTime;
            if (messageTimer > 0.0f) messageTimer -= deltaTime;

            spatialGrid.clear();
            if (enableSpatialGrid && currentState == PLAYING) {
                for (Entity e : enemyList) {
                    if (ecs.getEnemyAI(e).isAlive) {
                        Transform& t = ecs.getTransform(e);
                        spatialGrid.insert(e, t.x, t.z);
                    }
                }
            }

            if (currentState == COUNTDOWN) {
                countdownTimer -= deltaTime;
                currentSecond = (int)countdownTimer;
                if (countdownTimer <= -1.0f) currentState = PLAYING;
            } 
            else if (currentState == PLAYING) {
                
                if (currentMode == CLASSIC) {
                    bool anyAlive = false;
                    for (Entity e : enemyList) {
                        if (ecs.getEnemyAI(e).isAlive) { anyAlive = true; break; }
                    }
                    if (!anyAlive) {
                        currentWave++;
                        currentScore += 500; 
                        inGameMessage = "WAVE " + std::to_string(currentWave) + " STARTED!";
                        messageTimer = 2.0f;
                        spawnEnemies(currentWave * 10);
                        playerHealth = std::min(playerHealth + 2, 10); 
                    }
                }

                float moveSpeed = 15.0f; 
                marioPhysics.velocityX = 0.0f; marioPhysics.velocityZ = 0.0f; 
                if (myWindow.isKeyPressed(GLFW_KEY_D)) { marioPhysics.velocityX = moveSpeed; marioTransform.rotationY = 90.0f; }
                if (myWindow.isKeyPressed(GLFW_KEY_A)) { marioPhysics.velocityX = -moveSpeed; marioTransform.rotationY = -90.0f; }
                if (myWindow.isKeyPressed(GLFW_KEY_S)) { marioPhysics.velocityZ = moveSpeed; marioTransform.rotationY = 0.0f; }
                if (myWindow.isKeyPressed(GLFW_KEY_W)) { marioPhysics.velocityZ = -moveSpeed; marioTransform.rotationY = 180.0f; }
                
                marioTransform.x += marioPhysics.velocityX * deltaTime;
                marioTransform.z += marioPhysics.velocityZ * deltaTime;

                if (marioTransform.x < -99.5f) marioTransform.x = -99.5f; if (marioTransform.x >  99.5f) marioTransform.x =  99.5f;
                if (marioTransform.z < -99.5f) marioTransform.z = -99.5f; if (marioTransform.z >  99.5f) marioTransform.z =  99.5f;

                if (marioPhysics.velocityY > 0 && !myWindow.isKeyPressed(GLFW_KEY_SPACE)) marioPhysics.velocityY -= 50.0f * deltaTime; 
                else marioPhysics.velocityY -= 20.0f * deltaTime; 
                
                marioTransform.y += marioPhysics.velocityY * deltaTime;
                if (marioTransform.y <= 0.5f) { marioTransform.y = 0.5f; marioPhysics.velocityY = 0.0f; marioPhysics.isGrounded = true; }
                if (myWindow.isKeyPressed(GLFW_KEY_SPACE) && marioPhysics.isGrounded) { marioPhysics.velocityY = 12.0f; marioPhysics.isGrounded = false; }

                for (Entity e : enemyList) {
                    EnemyAI& ai = ecs.getEnemyAI(e);
                    if (!ai.isAlive) continue;

                    Transform& enemyTransform = ecs.getTransform(e);
                    PhysicsBody& enemyPhysics = ecs.getPhysicsBody(e); // NEW: Accessing the enemy's hidden physics!

                    float distX = marioTransform.x - enemyTransform.x;
                    float distZ = marioTransform.z - enemyTransform.z;
                    float distToPlayer = std::sqrt((distX * distX) + (distZ * distZ));

                    // X/Z Axis Movement
                    if (distToPlayer > 0.1f) {
                        enemyTransform.x += (distX / distToPlayer) * globalEnemySpeed * deltaTime;
                        enemyTransform.z += (distZ / distToPlayer) * globalEnemySpeed * deltaTime;
                    }

                    // NEW: Y Axis Movement (Gravity & Jumping)
                    enemyPhysics.velocityY -= 20.0f * deltaTime; // Apply gravity to enemy
                    enemyTransform.y += enemyPhysics.velocityY * deltaTime;

                    if (enemyTransform.y <= 0.5f) {
                        enemyTransform.y = 0.5f;
                        enemyPhysics.velocityY = 0.0f;
                        
                        // If jumping is toggled on, enemies have a probabilistic chance to jump per frame
                        if (enableEnemyJumping) {
                            if ((static_cast<float>(rand()) / RAND_MAX) < (0.6f * deltaTime)) { // ~0.6 jumps per second per enemy
                                enemyPhysics.velocityY = 12.0f; 
                            }
                        }
                    }

                    float sepX = 0.0f; float sepZ = 0.0f;
                    std::vector<Entity> neighborsToAvoid;
                    
                    if (enableSpatialGrid) {
                        neighborsToAvoid = spatialGrid.getNearbyEntities(enemyTransform.x, enemyTransform.z, 1.5f);
                    } else {
                        neighborsToAvoid = enemyList; 
                    }

                    for (Entity other : neighborsToAvoid) {
                        if (other == e || !ecs.getEnemyAI(other).isAlive) continue;
                        Transform& otherT = ecs.getTransform(other);
                        float dX = enemyTransform.x - otherT.x;
                        float dZ = enemyTransform.z - otherT.z;
                        float dSq = dX*dX + dZ*dZ;
                        
                        if (dSq > 0.001f && dSq < 2.25f) { 
                            float dist = std::sqrt(dSq);
                            sepX += (dX / dist);
                            sepZ += (dZ / dist);
                        }
                    }
                    enemyTransform.x += sepX * deltaTime * 3.5f; 
                    enemyTransform.z += sepZ * deltaTime * 3.5f;

                    // Combat logic naturally handles the new height differences!
                    if (distToPlayer < 1.0f && std::abs(marioTransform.y - enemyTransform.y) < 1.5f) {
                        if (marioPhysics.velocityY < 0.0f && marioTransform.y > enemyTransform.y + 0.3f) {
                            ai.isAlive = false;          
                            marioPhysics.velocityY = 10.0f; 
                            currentScore += 100;
                            inGameMessage = "Stomp! +100";
                            messageTimer = 1.0f;
                        } 
                        else if (damageCooldown <= 0.0f) {
                            playerHealth -= 1;
                            damageCooldown = 1.5f; 
                            inGameMessage = "Ouch!";
                            messageTimer = 1.0f;
                            if (playerHealth <= 0) { 
                                currentState = GAME_OVER; 
                                showDebugUI = false; 
                            }
                        }
                    }
                }

                if (currentMode == PRACTICE) {
                    float winDistX = marioTransform.x - flagX; float winDistZ = marioTransform.z - flagZ;
                    if (std::sqrt((winDistX * winDistX) + (winDistZ * winDistZ)) < 1.5f && marioTransform.y <= 5.0f) {
                        marioTransform.x = flagX; marioTransform.z = flagZ;
                        marioPhysics.velocityX = 0.0f; marioPhysics.velocityZ = 0.0f; marioPhysics.velocityY = 0.0f;
                        marioTransform.rotationY = 90.0f; 
                        currentState = FLAG_ANIMATION; 
                        showDebugUI = false;
                    }
                }
            }
            else if (currentState == FLAG_ANIMATION) {
                if (flagHeight > 0.0f) flagHeight -= 4.0f * deltaTime;
                if (marioTransform.y > 0.5f) marioTransform.y -= 4.0f * deltaTime;
                if (flagHeight <= 0.0f && marioTransform.y <= 0.5f) {
                    flagHeight = 0.0f; marioTransform.y = 0.5f; currentState = STAGE_COMPLETED;
                }
            }
        } 

        // --- IMGUI MENUS & UI ---
        if (currentState == MAIN_MENU) {
            ImGui::SetNextWindowPos(ImVec2(1440.0f / 2.0f, 900.0f / 2.0f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::Begin("Main Menu", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);
            ImGui::SetWindowFontScale(2.5f);
            ImGui::Text("SWARM SURVIVOR ENGINE");
            ImGui::SetWindowFontScale(1.5f);
            ImGui::Dummy(ImVec2(0.0f, 20.0f)); 
            if (ImGui::Button("Classic Mode (Survival)", ImVec2(400, 60))) { resetGame(CLASSIC); }
            ImGui::Dummy(ImVec2(0.0f, 5.0f)); 
            if (ImGui::Button("Practice Mode (Sandbox)", ImVec2(400, 60))) { resetGame(PRACTICE); }
            ImGui::Dummy(ImVec2(0.0f, 5.0f)); 
            if (ImGui::Button("Quit to Desktop", ImVec2(400, 60))) { glfwSetWindowShouldClose(myWindow.getRawWindow(), true); }
            ImGui::End();
        }

        if (currentState == PAUSED) {
            ImGui::SetNextWindowPos(ImVec2(1440.0f / 2.0f, 900.0f / 2.0f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::Begin("Pause Menu", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);
            ImGui::SetWindowFontScale(2.5f);
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "GAME PAUSED");
            ImGui::SetWindowFontScale(1.5f);
            ImGui::Dummy(ImVec2(0.0f, 20.0f));
            if (ImGui::Button("Resume (ESC)", ImVec2(300, 50))) { currentState = PLAYING; }
            ImGui::Dummy(ImVec2(0.0f, 5.0f)); 
            if (ImGui::Button("Return to Menu", ImVec2(300, 50))) { currentState = MAIN_MENU; }
            ImGui::End();
        }

        if (currentState == GAME_OVER || currentState == STAGE_COMPLETED) {
            ImGui::SetNextWindowPos(ImVec2(1440.0f / 2.0f, 900.0f / 2.0f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::Begin("End Screen", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);
            ImGui::SetWindowFontScale(2.5f);
            
            if (currentState == GAME_OVER) { ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "GAME OVER"); }
            if (currentState == STAGE_COMPLETED) { ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "STAGE COMPLETED!"); }
            
            ImGui::SetWindowFontScale(1.5f);
            if (currentMode == CLASSIC) { ImGui::Text("Survived to Wave: %d", currentWave); }
            ImGui::Text("Final Score: %d", currentScore);
            ImGui::Dummy(ImVec2(0.0f, 20.0f));
            if (ImGui::Button("Restart", ImVec2(300, 50))) { resetGame(currentMode); }
            if (ImGui::Button("Return to Menu", ImVec2(300, 50))) { currentState = MAIN_MENU; }
            ImGui::End();
        }

        if (showDebugUI && (currentState == PLAYING || currentState == PAUSED) && currentMode == PRACTICE) {
            ImGui::Begin("Engine Control Panel (Press TAB to hide)");
            int activeEnemies = 0;
            for(Entity e : enemyList) { if(ecs.getEnemyAI(e).isAlive) activeEnemies++; }
            ImGui::Text("Active Entities: %d", activeEnemies + 1); 
            
            ImGui::Separator();
            ImGui::Text("AI Configuration");
            ImGui::SliderFloat("Enemy Speed", &globalEnemySpeed, 1.0f, 15.0f);
            ImGui::Checkbox("Enable Enemy Jumping", &enableEnemyJumping); // NEW: The Jump Toggle
            
            ImGui::Separator();
            ImGui::Text("Stress Testing");
            if (ImGui::Button("Spawn 1 Enemy")) { spawnEnemies(1); }
            ImGui::SameLine();
            if (ImGui::Button("Spawn 100")) { spawnEnemies(100); }
            ImGui::SameLine();
            if (ImGui::Button("Spawn 1,000")) { spawnEnemies(1000); }
            
            if (ImGui::Button("Kill All Enemies")) {
                for (Entity e : enemyList) { ecs.getEnemyAI(e).isAlive = false; }
            }
            
            ImGui::Separator();
            ImGui::Text("Optimizations");
            ImGui::Checkbox("Enable Spatial Hash Grid", &enableSpatialGrid);
            if (!enableSpatialGrid) { ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "WARNING: O(N^2) Math active."); }
            ImGui::End();
        }

        if (currentState == PLAYING || currentState == PAUSED || currentState == FLAG_ANIMATION) {
            ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_Always);
            ImGui::Begin("Scoreboard", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs);
            ImGui::SetWindowFontScale(1.8f);
            if (currentMode == CLASSIC) {
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "MODE: CLASSIC");
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "WAVE: %d", currentWave);
            } else {
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "MODE: PRACTICE");
            }
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "SCORE: %d", currentScore);
            ImGui::End();
            
            ImGui::SetNextWindowPos(ImVec2(20.0f, 800.0f), ImGuiCond_Always);
            ImGui::Begin("InstructionsOverlay", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs);
            ImGui::SetWindowFontScale(1.2f);
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 0.8f), "Press ESC to Pause");
            if(currentMode == PRACTICE) ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 0.8f), "Press TAB to toggle Debug UI");
            ImGui::End();
        }

        if (currentState != MAIN_MENU) {
            ImGui::SetNextWindowPos(ImVec2(1420.0f, 20.0f), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
            ImGui::Begin("FPSOverlay", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs);
            ImGui::SetWindowFontScale(1.5f);
            ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "FPS: %.1f", io.Framerate);
            ImGui::End();
        }

        if ((currentState == PLAYING || currentState == PAUSED) && messageTimer > 0.0f) {
            ImGui::SetNextWindowPos(ImVec2(1440.0f / 2.0f, 250.0f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::Begin("GameMessageOverlay", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs);
            ImGui::SetWindowFontScale(3.0f);
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", inGameMessage.c_str());
            ImGui::End();
        }

        // --- RENDER UPDATE ---
        glClearColor(0.1f, 0.3f, 0.4f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (currentState != MAIN_MENU) {
            glMatrixMode(GL_PROJECTION); glLoadIdentity(); glFrustum(-1.6, 1.6, -1.0, 1.0, 2.0, 200.0); 
            glMatrixMode(GL_MODELVIEW); glLoadIdentity();
            glRotatef(25.0f, 1.0f, 0.0f, 0.0f); 
            glTranslatef(-marioTransform.x, -8.0f, -18.0f - marioTransform.z); 

            // NEW: Draw Shadows *before* the characters, slightly above the floor
            if (currentState != GAME_OVER) {
                bool isBlinking = (damageCooldown > 0.0f) && (sin(currentTime * 30.0f) > 0.0f);
                if (!isBlinking) {
                    glPushMatrix();
                    glTranslatef(marioTransform.x, 0.02f, marioTransform.z);
                    DrawShadow(0.6f);
                    glPopMatrix();
                }
            }
            
            for (Entity e : enemyList) {
                if (ecs.getEnemyAI(e).isAlive) {
                    Transform& t = ecs.getTransform(e);
                    glPushMatrix();
                    glTranslatef(t.x, 0.02f, t.z);
                    DrawShadow(0.7f);
                    glPopMatrix();
                }
            }

            glEnable(GL_LIGHTING);

            DrawPlatform();
            
            if (currentMode == PRACTICE) {
                glPushMatrix(); glTranslatef(flagX, 0.0f, flagZ); DrawFlagpole(flagHeight); glPopMatrix();
            }

            for (Entity e : enemyList) {
                if (ecs.getEnemyAI(e).isAlive) {
                    Transform& t = ecs.getTransform(e);
                    glPushMatrix(); glTranslatef(t.x, t.y, t.z); DrawEnemy(); glPopMatrix();
                }
            }

            if (currentState != GAME_OVER) {
                bool isBlinking = (damageCooldown > 0.0f) && (sin(currentTime * 30.0f) > 0.0f);
                if (!isBlinking) {
                    glPushMatrix();
                    glTranslatef(marioTransform.x, marioTransform.y, marioTransform.z);
                    glRotatef(marioTransform.rotationY, 0.0f, 1.0f, 0.0f); 
                    
                    glPushMatrix();
                    glTranslatef(0.0f, 0.4f, 0.5f);
                    glScalef(0.1f, 0.1f, 0.1f);
                    DrawUnitCube(1.0f, 0.0f, 0.0f);
                    glPopMatrix();

                    DrawPlayer();
                    glPopMatrix();
                }
            }

            glDisable(GL_LIGHTING);

            glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity(); glOrtho(0.0, 1440.0, 900.0, 0.0, -1.0, 1.0); 
            glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity(); glDisable(GL_DEPTH_TEST); 

            float barWidth = 400.0f;
            float barHeight = 30.0f;
            float startX = (1440.0f / 2.0f) - (barWidth / 2.0f);
            float startY = 900.0f - barHeight - 20.0f; 

            glBegin(GL_QUADS); glColor3f(0.3f, 0.0f, 0.0f);
            glVertex2f(startX, startY); glVertex2f(startX + barWidth, startY); glVertex2f(startX + barWidth, startY + barHeight); glVertex2f(startX, startY + barHeight); glEnd();

            glBegin(GL_QUADS); glColor3f(0.0f, 1.0f, 0.0f);
            float healthWidth = barWidth * ((float)playerHealth / 10.0f);
            if (healthWidth > 0.0f) {
                glVertex2f(startX, startY); glVertex2f(startX + healthWidth, startY); glVertex2f(startX + healthWidth, startY + barHeight); glVertex2f(startX, startY + barHeight);
            }
            glEnd();

            if (currentState == COUNTDOWN) {
                float cX = (1440.0f / 2.0f) - 60.0f; float cY = (900.0f / 2.0f) - 100.0f; 
                if (currentSecond > 0 && currentSecond <= 3) DrawRetroText(currentSecond, cX, cY);
                else if (currentSecond == 0) DrawRetroText(0, cX, cY); 
            }

            glEnable(GL_DEPTH_TEST); glMatrixMode(GL_MODELVIEW); glPopMatrix(); glMatrixMode(GL_PROJECTION); glPopMatrix();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        myWindow.swapBuffers();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    return 0;
}