#include "Engine/Window.h"
#include "Engine/Core/ECS.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <ctime>

enum GameState { COUNTDOWN, PLAYING, FLAG_ANIMATION, STAGE_COMPLETED, GAME_OVER };

// --- 3D DRAWING HELPERS ---
void DrawUnitCube(float r, float g, float b) {
    glBegin(GL_QUADS);
    glColor3f(r, g, b); 
    glVertex3f(-0.5f, -0.5f,  0.5f); glVertex3f( 0.5f, -0.5f,  0.5f); glVertex3f( 0.5f,  0.5f,  0.5f); glVertex3f(-0.5f,  0.5f,  0.5f);
    glColor3f(r*0.8f, g*0.8f, b*0.8f); 
    glVertex3f( 0.5f, -0.5f, -0.5f); glVertex3f( 0.5f,  0.5f, -0.5f); glVertex3f( 0.5f,  0.5f,  0.5f); glVertex3f( 0.5f, -0.5f,  0.5f);
    glColor3f(r*0.9f, g*0.9f, b*0.9f); 
    glVertex3f(-0.5f,  0.5f, -0.5f); glVertex3f(-0.5f,  0.5f,  0.5f); glVertex3f( 0.5f,  0.5f,  0.5f); glVertex3f( 0.5f,  0.5f, -0.5f);
    glColor3f(r*0.7f, g*0.7f, b*0.7f); 
    glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f(-0.5f, -0.5f,  0.5f); glVertex3f(-0.5f,  0.5f,  0.5f); glVertex3f(-0.5f,  0.5f, -0.5f);
    glColor3f(r*0.6f, g*0.6f, b*0.6f); 
    glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f( 0.5f, -0.5f, -0.5f); glVertex3f( 0.5f,  0.5f, -0.5f); glVertex3f(-0.5f,  0.5f, -0.5f);
    glColor3f(r*0.5f, g*0.5f, b*0.5f); 
    glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f( 0.5f, -0.5f, -0.5f); glVertex3f( 0.5f, -0.5f,  0.5f); glVertex3f(-0.5f, -0.5f,  0.5f);
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

// NEW: Goomba-style blocky enemy
void DrawEnemy() {
    glPushMatrix(); glTranslatef(0.0f, 0.0f, 0.0f); glScalef(0.8f, 0.6f, 0.8f); DrawUnitCube(0.4f, 0.2f, 0.0f); glPopMatrix(); // Brown body
    glPushMatrix(); glTranslatef(0.0f, 0.4f, 0.0f); glScalef(0.9f, 0.3f, 0.9f); DrawUnitCube(0.6f, 0.3f, 0.1f); glPopMatrix(); // Mushroom head
}

void DrawPlatform() {
    glBegin(GL_QUADS);
    glColor3f(0.0f, 0.6f, 0.2f); glVertex3f(-30.0f, 0.0f,  30.0f); glVertex3f( 30.0f, 0.0f,  30.0f); glVertex3f( 30.0f, 0.0f, -30.0f); glVertex3f(-30.0f, 0.0f, -30.0f);
    glColor3f(0.3f, 0.2f, 0.1f); glVertex3f(-30.0f, -5.0f,  30.0f); glVertex3f( 30.0f, -5.0f,  30.0f); glVertex3f( 30.0f, -5.0f, -30.0f); glVertex3f(-30.0f, -5.0f, -30.0f);
    glEnd();
}

void DrawFlagpole(float currentFlagHeight) {
    glBegin(GL_QUADS);
    glColor3f(0.8f, 0.8f, 0.0f); glVertex3f(-0.1f, 0.0f, 0.1f); glVertex3f( 0.1f, 0.0f, 0.1f); glVertex3f( 0.1f, 5.0f, 0.1f); glVertex3f(-0.1f, 5.0f, 0.1f);
    glColor3f(1.0f, 1.0f, 1.0f); glVertex3f( 0.1f, currentFlagHeight, 0.0f); glVertex3f( 1.5f, currentFlagHeight, 0.0f); glVertex3f( 1.5f, currentFlagHeight + 1.0f, 0.0f); glVertex3f( 0.1f, currentFlagHeight + 1.0f, 0.0f);
    glEnd();
}

// --- 2D HUD HELPERS ---
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
    srand(static_cast<unsigned int>(time(0))); // Seed for random enemy spawns
    Window myWindow(1440, 900, "VIE-Project Engine");
    Registry ecs;

    // Player Init
    Entity mario = ecs.createEntity();
    ecs.getTransform(mario).y = 8.0f;
    
    // NEW: Spawn 6 Enemies randomly across the map
    std::vector<Entity> enemyList;
    for (int i = 0; i < 6; i++) {
        Entity e = ecs.createEntity();
        Transform& t = ecs.getTransform(e);
        EnemyAI& ai = ecs.getEnemyAI(e);
        
        // Random spot between -20 and 20
        t.x = (rand() % 40) - 20.0f;
        t.z = (rand() % 40) - 20.0f;
        t.y = 0.5f; 
        
        ai.spawnX = t.x;
        ai.spawnZ = t.z;
        ai.isAlive = true;
        enemyList.push_back(e);
    }

    GameState currentState = COUNTDOWN;
    float flagX = 28.0f; float flagZ = -28.0f; float flagHeight = 4.0f; 
    
    // Updated Player Stats
    int playerHealth = 10; 
    int currentScore = 0;
    float damageCooldown = 0.0f; // I-Frames timer
    
    float countdownTimer = 3.99f;
    int currentSecond = 4; 

    float lastTime = myWindow.getTime();
    glEnable(GL_DEPTH_TEST);

    while (!myWindow.shouldClose()) {
        float currentTime = myWindow.getTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        myWindow.pollEvents();
        if (myWindow.isKeyPressed(GLFW_KEY_ESCAPE)) break;

        Transform& marioTransform = ecs.getTransform(mario);
        PhysicsBody& marioPhysics = ecs.getPhysicsBody(mario);

        if (damageCooldown > 0.0f) damageCooldown -= deltaTime;

        if (currentState == COUNTDOWN) {
            countdownTimer -= deltaTime;
            currentSecond = (int)countdownTimer;
            if (countdownTimer <= -1.0f) currentState = PLAYING;
        } 
        else if (currentState == PLAYING) {
            // --- PLAYER LOGIC ---
            float moveSpeed = 15.0f; 
            marioPhysics.velocityX = 0.0f; marioPhysics.velocityZ = 0.0f; 
            if (myWindow.isKeyPressed(GLFW_KEY_D)) { marioPhysics.velocityX = moveSpeed; marioTransform.rotationY = 90.0f; }
            if (myWindow.isKeyPressed(GLFW_KEY_A)) { marioPhysics.velocityX = -moveSpeed; marioTransform.rotationY = -90.0f; }
            if (myWindow.isKeyPressed(GLFW_KEY_S)) { marioPhysics.velocityZ = moveSpeed; marioTransform.rotationY = 0.0f; }
            if (myWindow.isKeyPressed(GLFW_KEY_W)) { marioPhysics.velocityZ = -moveSpeed; marioTransform.rotationY = 180.0f; }
            
            marioTransform.x += marioPhysics.velocityX * deltaTime;
            marioTransform.z += marioPhysics.velocityZ * deltaTime;

            // Boundaries
            if (marioTransform.x < -29.5f) marioTransform.x = -29.5f; if (marioTransform.x >  29.5f) marioTransform.x =  29.5f;
            if (marioTransform.z < -29.5f) marioTransform.z = -29.5f; if (marioTransform.z >  29.5f) marioTransform.z =  29.5f;

            // Gravity & Jump
            if (marioPhysics.velocityY > 0 && !myWindow.isKeyPressed(GLFW_KEY_SPACE)) marioPhysics.velocityY -= 50.0f * deltaTime; 
            else marioPhysics.velocityY -= 20.0f * deltaTime; 
            
            marioTransform.y += marioPhysics.velocityY * deltaTime;
            if (marioTransform.y <= 0.5f) { marioTransform.y = 0.5f; marioPhysics.velocityY = 0.0f; marioPhysics.isGrounded = true; }
            if (myWindow.isKeyPressed(GLFW_KEY_SPACE) && marioPhysics.isGrounded) { marioPhysics.velocityY = 12.0f; marioPhysics.isGrounded = false; }

            // --- ENEMY AI & COLLISION LOGIC ---
            for (Entity e : enemyList) {
                EnemyAI& ai = ecs.getEnemyAI(e);
                if (!ai.isAlive) continue; // Skip dead enemies

                Transform& enemyTransform = ecs.getTransform(e);
                
                // Calculate distance to player
                float distX = marioTransform.x - enemyTransform.x;
                float distZ = marioTransform.z - enemyTransform.z;
                float distToPlayer = std::sqrt((distX * distX) + (distZ * distZ));

                // 1. Movement AI
                float enemySpeed = 4.0f;
                if (distToPlayer <= 5.0f) {
                    // Aggro! Chase the player
                    enemyTransform.x += (distX / distToPlayer) * enemySpeed * deltaTime;
                    enemyTransform.z += (distZ / distToPlayer) * enemySpeed * deltaTime;
                } else {
                    // Wander slowly in a circle around their spawn point
                    float wanderSpeed = 1.5f;
                    float targetX = ai.spawnX + std::cos(currentTime + e) * 3.0f; // e offsets their phase so they don't move in unison
                    float targetZ = ai.spawnZ + std::sin(currentTime + e) * 3.0f;
                    float wDistX = targetX - enemyTransform.x;
                    float wDistZ = targetZ - enemyTransform.z;
                    float wLen = std::sqrt((wDistX * wDistX) + (wDistZ * wDistZ));
                    if (wLen > 0.1f) {
                        enemyTransform.x += (wDistX / wLen) * wanderSpeed * deltaTime;
                        enemyTransform.z += (wDistZ / wLen) * wanderSpeed * deltaTime;
                    }
                }

                // 2. Combat Collision
                if (distToPlayer < 1.0f && std::abs(marioTransform.y - enemyTransform.y) < 1.5f) {
                    // Are we falling on their head?
                    if (marioPhysics.velocityY < 0.0f && marioTransform.y > enemyTransform.y + 0.3f) {
                        ai.isAlive = false;          // Kill Enemy
                        marioPhysics.velocityY = 10.0f; // Bounce Mario back up!
                        currentScore += 100;
                        std::cout << "Stomp! Score: " << currentScore << std::endl;
                    } 
                    // Otherwise, we got hit!
                    else if (damageCooldown <= 0.0f) {
                        playerHealth -= 1;
                        damageCooldown = 1.5f; // 1.5 seconds of invincibility
                        std::cout << "Ouch! HP: " << playerHealth << "/10" << std::endl;
                        
                        if (playerHealth <= 0) {
                            currentState = GAME_OVER;
                            std::cout << "--- GAME OVER ---" << std::endl;
                        }
                    }
                }
            }

            // --- WIN CONDITION CHECK ---
            float winDistX = marioTransform.x - flagX; float winDistZ = marioTransform.z - flagZ;
            if (std::sqrt((winDistX * winDistX) + (winDistZ * winDistZ)) < 1.5f && marioTransform.y <= 5.0f) {
                currentScore += (int)((marioTransform.y / 5.0f) * 5000);
                std::cout << "\n>>> FLAG CAPTURED! <<<" << std::endl;
                marioTransform.x = flagX; marioTransform.z = flagZ;
                marioPhysics.velocityX = 0.0f; marioPhysics.velocityZ = 0.0f; marioPhysics.velocityY = 0.0f;
                marioTransform.rotationY = 90.0f; 
                currentState = FLAG_ANIMATION; 
            }
        }
        else if (currentState == FLAG_ANIMATION) {
            if (flagHeight > 0.0f) flagHeight -= 4.0f * deltaTime;
            if (marioTransform.y > 0.5f) marioTransform.y -= 4.0f * deltaTime;
            if (flagHeight <= 0.0f && marioTransform.y <= 0.5f) {
                flagHeight = 0.0f; marioTransform.y = 0.5f; currentState = STAGE_COMPLETED;
                std::cout << "\n===============================\n        STAGE COMPLETED!       \n     Total Score: " << currentScore << " pts!\n===============================\n";
            }
        }

        // --- RENDER UPDATE ---
        glClearColor(0.2f, 0.6f, 1.0f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION); glLoadIdentity(); glFrustum(-1.6, 1.6, -1.0, 1.0, 2.0, 200.0); 
        glMatrixMode(GL_MODELVIEW); glLoadIdentity();
        glRotatef(25.0f, 1.0f, 0.0f, 0.0f); 
        glTranslatef(-marioTransform.x, -8.0f, -18.0f - marioTransform.z); 

        DrawPlatform();
        glPushMatrix(); glTranslatef(flagX, 0.0f, flagZ); DrawFlagpole(flagHeight); glPopMatrix();

        // Draw Enemies
        for (Entity e : enemyList) {
            if (ecs.getEnemyAI(e).isAlive) {
                Transform& t = ecs.getTransform(e);
                glPushMatrix();
                glTranslatef(t.x, t.y, t.z);
                DrawEnemy();
                glPopMatrix();
            }
        }

        // Draw Player (Only if NOT blinking from damage)
        if (currentState != GAME_OVER) {
            // Blinking logic: If I-frames are active, toggle drawing off every 0.1 seconds
            bool isBlinking = (damageCooldown > 0.0f) && (sin(currentTime * 30.0f) > 0.0f);
            if (!isBlinking) {
                glPushMatrix();
                glTranslatef(marioTransform.x, marioTransform.y, marioTransform.z);
                glRotatef(marioTransform.rotationY, 0.0f, 1.0f, 0.0f); 
                DrawPlayer();
                glPopMatrix();
            }
        }

        // --- 2D HUD RENDER LAYER ---
        glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity(); glOrtho(0.0, 1440.0, 900.0, 0.0, -1.0, 1.0); 
        glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity(); glDisable(GL_DEPTH_TEST); 

        // Draw Health Bar Outline
        glBegin(GL_QUADS); glColor3f(0.3f, 0.0f, 0.0f);
        glVertex2f(50.0f, 50.0f); glVertex2f(350.0f, 50.0f); glVertex2f(350.0f, 80.0f); glVertex2f(50.0f, 80.0f); glEnd();

        // Fill Health Bar (Scaled to 10 HP max)
        glBegin(GL_QUADS); glColor3f(0.0f, 1.0f, 0.0f);
        float healthWidth = 300.0f * ((float)playerHealth / 10.0f);
        if (healthWidth > 0.0f) {
            glVertex2f(50.0f, 50.0f); glVertex2f(50.0f + healthWidth, 50.0f); glVertex2f(50.0f + healthWidth, 80.0f); glVertex2f(50.0f, 80.0f);
        }
        glEnd();

        if (currentState == COUNTDOWN) {
            float cX = (1440.0f / 2.0f) - 60.0f; float cY = (900.0f / 2.0f) - 100.0f; 
            if (currentSecond > 0 && currentSecond <= 3) DrawRetroText(currentSecond, cX, cY);
            else if (currentSecond == 0) DrawRetroText(0, cX, cY); 
        }

        glEnable(GL_DEPTH_TEST); glMatrixMode(GL_MODELVIEW); glPopMatrix(); glMatrixMode(GL_PROJECTION); glPopMatrix();
        myWindow.swapBuffers();
    }
    return 0;
}