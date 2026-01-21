#ifndef HELPERS_H
#define HELPERS_H

#include "raylib.h"

typedef struct RenderSettings {
    int aaEnabled;
} RenderSettings;

float Clampf(float value, float min, float max);

bool Movement(Camera *camera);
void Zoom(Camera *camera);

void Settings(RenderSettings *settings);
void DrawInfo(Camera camera, RenderSettings settings, int frame);

void CopyTexture(RenderTexture source, RenderTexture target, float resolution[2]);
void ClearTexture(RenderTexture tex);

#endif
