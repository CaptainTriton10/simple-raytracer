#ifndef CONTROLS_H
#define CONTROLS_H

#include "../renderer/renderer.h"
#include <raylib.h>
#include <stdbool.h>

#define CAMERA_MOVE_SPEED 1.5
#define CAMERA_ZOOM_SPEED 7
#define MOUSE_SENSETIVITY 0.5

typedef struct BasisVectors {
    float forward[3];
    float right[3];
    float up[3];
} BasisVectors;

bool Movement(Camera *camera, BasisVectors vectors);
bool Zoom(Camera *camera, RenderSettings settings);
BasisVectors Look(float *yaw, float *pitch, bool isEnabled);

bool Settings(RenderSettings *settings);
void ToggleCursor(bool *isEnabled);

void DrawInfo(Camera camera, RenderSettings settings, int frame, Vector2 cameraRotation);

#endif
