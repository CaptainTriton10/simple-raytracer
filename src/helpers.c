#include "../include/helpers.h"
#include "raylib.h"
#include <stdio.h>
#include <math.h>

#define CAMERA_MOVE_SPEED 1.5
#define CAMERA_ZOOM_SPEED 4

float Clampf(float value, float min, float max) {
    return fmaxf(min, fminf(value, max));
}

bool Movement(Camera *camera) {
    float move = CAMERA_MOVE_SPEED * GetFrameTime();
    bool changed = false;

    if (IsKeyDown(KEY_W)) {
        camera->position.z += -move;
        changed = true;
    }

    if (IsKeyDown(KEY_A)) {
        camera->position.x += -move;
        changed = true;
    }

    if (IsKeyDown(KEY_S)) {
        camera->position.z += move;
        changed = true;
    }

    if (IsKeyDown(KEY_D)) {
        camera->position.x += move;
        changed = true;
    }

    if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_C)) {
        camera->position.y += -move;
        changed = true;
    }

    if (IsKeyDown(KEY_SPACE)) {
        camera->position.y += move;
        changed = true;
    }

    return changed;
}

void Zoom(Camera *camera) {
    float zoomFactor = CAMERA_ZOOM_SPEED * GetFrameTime();
    float scroll = 1 + zoomFactor * GetMouseWheelMove();

    camera->fovy = Clampf(camera->fovy * scroll, 0.1, 30);
}

void Settings(RenderSettings *settings) {
    if (IsKeyPressed(KEY_ONE)) {
        settings->aaEnabled = settings->aaEnabled == 1 ? 0 : 1;
    }
}

void DrawInfo(Camera camera, RenderSettings settings, int frame) {
    char frameInfo[16];
    sprintf(frameInfo, "Frame: %d", frame);

    char cameraPosInfo[128];
    sprintf(cameraPosInfo, "Camera Position: [%.2f, %.2f, %.2f]", camera.position.x, camera.position.y, camera.position.z);

    char cameraFovyInfo[64];
    sprintf(cameraFovyInfo, "Camera Focal Length: %.2f", camera.fovy);

    char aaInfo[64];
    sprintf(aaInfo, "Anti-Aliasing: %d", settings.aaEnabled);

    DrawFPS(5, 5);

    DrawText(cameraPosInfo, 5, 50, 20, RED);
    DrawText(cameraFovyInfo, 5, 75, 20, RED);

    DrawText(aaInfo, 5, 125, 20, YELLOW);

    DrawText(frameInfo, 5, 175, 20, PURPLE);
}

void CopyTexture(RenderTexture source, RenderTexture target, float resolution[2]) {
    BeginTextureMode(target);
        DrawTextureRec(
            source.texture,
            (Rectangle){ 0, 0, (float)resolution[0], -(float)resolution[1] },
            (Vector2){ 0, 0 },
            WHITE
        );
    EndTextureMode();
}

void ClearTexture(RenderTexture tex) {
    BeginTextureMode(tex);
        ClearBackground(BLACK);
    EndTextureMode();
}
