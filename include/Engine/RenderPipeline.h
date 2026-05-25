#ifndef RENDER_PIPELINE_H
#define RENDER_PIPELINE_H

#include "Engine/GameTypes.h"

void DrawProceduralCube(float baseR, float baseG, float baseB, float noiseFreq, bool isGrassTop = false);
void DrawPlayer(float r, float g, float b, int gunLevel, bool gunEnabled);
void DrawEnemy(float r, float g, float b);
void DrawPlatform();
void DrawTrapPrimitive(float sx, float sz);
void DrawFlatBoxShadow(float hX, float hZ);
void DrawCustomAsset(const EnvironmentBlock& block);
void DrawShadow(float radius);

#endif