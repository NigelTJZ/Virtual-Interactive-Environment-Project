#include "Engine/RenderPipeline.h"
#include <GLFW/glfw3.h>
#include <cmath>

// High performance procedural noise utility for terrain map generation
float GetProceduralNoise(float x, float z, float frequency) {
    float n = sin(x * frequency) * cos(z * frequency) + sin(x * frequency * 2.5f) * sin(z * frequency * 1.8f);
    return (n + 2.0f) / 4.0f; 
}

// Draw standard block space primitives with directional light normals
void DrawProceduralCube(float baseR, float baseG, float baseB, float noiseFreq, bool isGrassTop) {
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
    glVertex3f( 0.5f, -0.5f,  0.5f); glVertex3f( 0.5f, -0.5f, -0.5f); glVertex3f( 0.5f,  0.5f, -0.5f); glVertex3f( 0.5f,  0.5f,  0.5f);
    
    // BOTTOM FACE
    glNormal3f(0.0f, -1.0f, 0.0f); glColor3f(baseR * 0.6f, baseG * 0.6f, baseB * 0.6f);
    glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f( 0.5f, -0.5f, -0.5f); glVertex3f( 0.5f, -0.5f,  0.5f); glVertex3f(-0.5f, -0.5f,  0.5f);
    glEnd();
}

// Render player body matrix segments along with staggered side attachments
void DrawPlayer(float r, float g, float b, int gunLevel, bool gunEnabled) {
    glPushMatrix(); glTranslatef(0.0f, 0.1f, 0.0f); glScalef(0.7f, 0.9f, 0.7f); DrawProceduralCube(r, g, b, 0.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, 0.25f, 0.35f); glScalef(0.5f, 0.3f, 0.1f); DrawProceduralCube(0.9f, 0.9f, 1.0f, 0.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.15f, -0.35f, 0.0f); glScalef(0.25f, 0.3f, 0.25f); DrawProceduralCube(r, g, b, 0.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.15f, -0.35f, 0.0f); glScalef(0.25f, 0.3f, 0.25f); DrawProceduralCube(r, g, b, 0.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, 0.55f, 0.0f); glScalef(0.75f, 0.05f, 0.75f); DrawProceduralCube(r*1.1f, g, b, 0.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, 0.6f, 0.0f); glScalef(0.6f, 0.1f, 0.6f); DrawProceduralCube(1.0f, 1.0f, 1.0f, 0.0f); glPopMatrix();

    if (gunEnabled) {
        for (int i = 0; i < gunLevel; ++i) {
            glPushMatrix();
            float sideOffset = (i % 2 == 0) ? 0.45f : -0.45f;
            float heightRow = (i / 2) * 0.25f;
            glTranslatef(sideOffset, -0.1f + heightRow, 0.1f);
            glScalef(0.18f, 0.18f, 0.4f);
            DrawProceduralCube(1.0f, 0.85f, 0.0f, 0.0f); 
            glPopMatrix();
        }
    }
}

// Render standard enemy block configurations
void DrawEnemy(float r, float g, float b) {
    glPushMatrix(); glTranslatef(0.0f, 0.0f, 0.0f); glScalef(0.8f, 0.6f, 0.8f); DrawProceduralCube(r*0.6f, g*0.6f, b*0.6f, 0.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, 0.4f, 0.0f); glScalef(0.9f, 0.3f, 0.9f); DrawProceduralCube(r, g, b, 0.0f); glPopMatrix();
}

// Render dynamic ground tile canvas partitions
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

// Draw dangerous spike cluster assets via a coordinate generation loop
void DrawTrapPrimitive(float sx, float sz) {
    for (float dx = -sx * 0.4f; dx <= sx * 0.4f; dx += 0.4f) {
        for (float dz = -sz * 0.4f; dz <= sz * 0.4f; dz += 0.4f) {
            glPushMatrix(); glTranslatef(dx, 0.15f, dz); glScalef(0.12f, 0.35f, 0.12f); DrawProceduralCube(0.28f, 0.32f, 0.15f, 0.0f); glPopMatrix();
        }
    }
}

// Draw low-overhead flat gray shadow box markers on floor textures
void DrawFlatBoxShadow(float hX, float hZ) {
    glDisable(GL_LIGHTING); glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); glDepthMask(GL_FALSE);
    glColor4f(0.15f, 0.15f, 0.15f, 0.75f); 
    glBegin(GL_QUADS);
    glVertex3f(-hX * 0.5f, 0.01f,  hZ * 0.5f);
    glVertex3f( hX * 0.5f, 0.01f,  hZ * 0.5f);
    glVertex3f( hX * 0.5f, 0.01f, -hZ * 0.5f);
    glVertex3f(-hX * 0.5f, 0.01f, -hZ * 0.5f);
    glEnd();
    glDepthMask(GL_TRUE); glDisable(GL_BLEND); glEnable(GL_LIGHTING);
}

// Process structural geometry definitions for item models, flora, and goals
void DrawCustomAsset(const EnvironmentBlock& block) {
    if (block.type == ASSET_BLOCK) {
        glPushMatrix(); glTranslatef(block.x, block.y, block.z); glScalef(block.scaleX, block.scaleY, block.scaleZ);
        DrawProceduralCube(0.88f, 0.55f, 0.12f, 0.0f);
        glPopMatrix();

        glPushMatrix(); glDisable(GL_LIGHTING); glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); glDepthMask(GL_FALSE);
        glColor4f(0.15f, 0.15f, 0.15f, 0.75f);
        glTranslatef(block.x, 0.01f, block.z); 
        glScalef(block.scaleX * 0.95f, 0.01f, block.scaleZ * 0.95f);
        DrawProceduralCube(0.15f, 0.15f, 0.15f, 0.0f);
        glDepthMask(GL_TRUE); glDisable(GL_BLEND); glEnable(GL_LIGHTING); glPopMatrix();
    }
    else if (block.type == ASSET_ROCK) {
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
        glPushMatrix(); glTranslatef(block.x, block.y, block.z); DrawTrapPrimitive(2.5f, 2.5f); glPopMatrix();
    }
    else if (block.type == ASSET_GOAL) {
        glPushMatrix(); glTranslatef(block.x, block.y + 2.0f, block.z); glScalef(0.15f, 4.0f, 0.15f); DrawProceduralCube(0.85f, 0.65f, 0.15f, 0.0f); glPopMatrix();
        glPushMatrix(); glTranslatef(block.x + 0.45f, block.y + 3.6f, block.z); glScalef(0.8f, 0.5f, 0.1f); DrawProceduralCube(0.9f, 0.9f, 0.9f, 0.0f); glPopMatrix();
    }
}

// Project circular translucent shadow quads onto floor surfaces
void DrawShadow(float radius) {
    glDisable(GL_LIGHTING); glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); glDepthMask(GL_FALSE); glColor4f(0.0f, 0.0f, 0.0f, 0.4f); 
    glBegin(GL_QUADS); glVertex3f(-radius, 0.02f, radius); glVertex3f(radius, 0.02f, radius); glVertex3f(radius, 0.02f, -radius); glVertex3f(-radius, 0.02f, -radius); glEnd();
    glDepthMask(GL_TRUE); glDisable(GL_BLEND); glEnable(GL_LIGHTING);
}