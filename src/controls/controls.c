#include "controls.h"
#include "../utils/utils.h"
#include "raylib.h"
#include <stdio.h>
#include <raymath.h>
#include <string.h>

bool Movement(Camera *camera, BasisVectors vectors) {
    float move = CAMERA_MOVE_SPEED * GetFrameTime();
    bool changed = false;

    if (IsKeyDown(KEY_W)) {
        camera->position.x += vectors.forward[0] * move;
        camera->position.y += vectors.forward[1] * move;
        camera->position.z += vectors.forward[2] * move;
        changed = true;
    }

    if (IsKeyDown(KEY_S)) {
        camera->position.x -= vectors.forward[0] * move;
        camera->position.y -= vectors.forward[1] * move;
        camera->position.z -= vectors.forward[2] * move;
        changed = true;
    }

    if (IsKeyDown(KEY_A)) {
        camera->position.x -= vectors.right[0] * move;
        camera->position.y -= vectors.right[1] * move;
        camera->position.z -= vectors.right[2] * move;
        changed = true;
    }

    if (IsKeyDown(KEY_D)) {
        camera->position.x += vectors.right[0] * move;
        camera->position.y += vectors.right[1] * move;
        camera->position.z += vectors.right[2] * move;
        changed = true;
    }

    if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_C)) {
        camera->position.x -= vectors.up[0] * move;
        camera->position.y -= vectors.up[1] * move;
        camera->position.z -= vectors.up[2] * move;
        changed = true;
    }

    if (IsKeyDown(KEY_SPACE)) {
        camera->position.x += vectors.up[0] * move;
        camera->position.y += vectors.up[1] * move;
        camera->position.z += vectors.up[2] * move;
        changed = true;
    }

    return changed;
}

bool Zoom(Camera *camera, RenderSettings settings) {
    float zoomFactor = CAMERA_ZOOM_SPEED * GetFrameTime();
    float scroll = 1 + zoomFactor * GetMouseWheelMove();

    camera->fovy = Clamp(camera->fovy / scroll, settings.fovLimits[0], settings.fovLimits[1]);

    // If the camera was zoomed this frame
    if (scroll != 1.0) {
        return true;
    }

    return false;
}

BasisVectors Look(float *yaw, float *pitch, bool isEnabled) {
    BasisVectors vectors;
    float worldUp[3] = {0.0, 1.0, 0.0};

    Vector2 mouseDelta = {
            GetMouseDelta().x * MOUSE_SENSETIVITY * GetFrameTime(),
            GetMouseDelta().y * MOUSE_SENSETIVITY * GetFrameTime()};

    *yaw -= isEnabled ? 0.0f : mouseDelta.x;
    *pitch -= isEnabled ? 0.0f : mouseDelta.y;
    *pitch = Clamp(*pitch, -85.0f * DEG2RAD, 85.0f * DEG2RAD);

    float forward[3] = {
        cosf(*pitch) * cosf(*yaw),
        sinf(*pitch),
        cosf(*pitch) * sinf(*yaw)
    };

    NormaliseVec3(forward);

    float right[3];
    CrossVec3(right, worldUp, forward);
    NormaliseVec3(right);

    float up[3];
    CrossVec3(up, forward, right);

    memcpy(vectors.forward, forward, sizeof(forward));
    memcpy(vectors.right, right, sizeof(right));
    memcpy(vectors.up, up, sizeof(up));

    return vectors;
}

bool Settings(RenderSettings *settings) {
    if (IsKeyPressed(KEY_ONE)) {
        // settings->aaEnabled = settings->aaEnabled == 1 ? 0 : 1;
        return true;
    }

    return false;
}

void ToggleCursor(bool *isEnabled) {
    if (IsKeyPressed(KEY_LEFT_ALT)) {
        if (*isEnabled) DisableCursor();
        else EnableCursor();

        *isEnabled = !*isEnabled;
    }
}

void DrawInfo(Camera camera, RenderSettings settings, int frame, Vector2 cameraRotation) {
    char frameInfo[16];
    sprintf(frameInfo, "Frame: %d", frame);

    char cameraPosInfo[128];
    sprintf(cameraPosInfo, "Camera Position: [%.2f, %.2f, %.2f]", camera.position.x, camera.position.y, camera.position.z);

    char cameraRotInfo[128];
    sprintf(cameraRotInfo, "Camera Rotation: [yaw = %.2f, pitch = %.2f]", cameraRotation.x * RAD2DEG, cameraRotation.y * RAD2DEG);

    char cameraFovyInfo[64];
    sprintf(cameraFovyInfo, "Camera Focal Length: %.2f", camera.fovy);

    char aaInfo[64];
    sprintf(aaInfo, "Anti-Aliasing: %d", settings.aaEnabled);

    DrawFPS(5, 5);

    DrawText(cameraPosInfo, 5, 50, 20, RED);
    DrawText(cameraRotInfo, 5, 75, 20, RED);
    DrawText(cameraFovyInfo, 5, 100, 20, RED);

    DrawText(aaInfo, 5, 150, 20, YELLOW);

    DrawText(frameInfo, 5, 200, 20, PURPLE);
}
