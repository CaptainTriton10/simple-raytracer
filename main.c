#include "raylib.h"
#include <stdio.h>

// On Windows, target dedicated GPU with NVIDIA Optimus and AMD PowerXpress/Switchable Graphics
#ifdef _WIN32
    #ifdef __cplusplus
    extern "C" {
    #endif

    // NVIDIA Optimus
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;

    // AMD PowerXpress/Switchable Graphics
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;

    #ifdef __cplusplus
    }
    #endif
#endif

#define CAMERA_MOVE_SPEED 1.5
#define CAMERA_ZOOM_SPEED 4

void Movement(Camera *camera) {
    float move = CAMERA_MOVE_SPEED * GetFrameTime();

    if (IsKeyDown(KEY_W)) {
        camera->position.z += -move;
    }

    if (IsKeyDown(KEY_A)) {
        camera->position.x += -move;
    }

    if (IsKeyDown(KEY_S)) {
        camera->position.z += move;
    }

    if (IsKeyDown(KEY_D)) {
        camera->position.x += move;
    }

    if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_C)) {
        camera->position.y += -move;
    }

    if (IsKeyDown(KEY_SPACE)) {
        camera->position.y += move;
    }
}

void Zoom(Camera *camera) {
    float zoomFactor = CAMERA_ZOOM_SPEED * GetFrameTime();
    float scroll = GetMouseWheelMove();

    if (camera->fovy > 0.1 && scroll < 0) {
        camera->fovy *= 1 + scroll * zoomFactor;
    }

    if (camera->fovy < 30 && scroll > 0) {
        camera->fovy *= 1 + scroll * zoomFactor;
    }
}

void DrawInfo(Camera camera) {
    // Camera position
    char cameraPosInfo[128];
    sprintf(cameraPosInfo, "Camera Position: [%.2f, %.2f, %.2f]", camera.position.x, camera.position.y, camera.position.z);

    char cameraFovyInfo[64];
    sprintf(cameraFovyInfo, "Camera Focal Length: %.2f", camera.fovy);

    DrawText(cameraPosInfo, 5, 25, 20, RED);
    DrawText(cameraFovyInfo, 5, 50, 20, RED);
}

int main(void) {
    const float aspectRatio = 2.0f / 1.0f;

    const int screenWidth = 1280;
    const int screenHeight = (int)(screenWidth / aspectRatio);

    InitWindow(screenWidth, screenHeight, "Simple Raytracer");

    Camera camera = {
        .position = {0.0f, 0.0f, 2.0f},
        .fovy = 2.0f
    };

    SetTargetFPS(100);

    Shader shader = LoadShader(0, "raytracing.frag");

    int timeLoc = GetShaderLocation(shader, "time");
    int resLoc  = GetShaderLocation(shader, "resolution");

    int flenLoc = GetShaderLocation(shader, "focalLength");
    int camCenLoc = GetShaderLocation(shader, "cameraCenter");
    int viewpLoc = GetShaderLocation(shader, "viewport");

    while (!WindowShouldClose()) {    // Detect window close button or ESC key


        Movement(&camera);
        Zoom(&camera);

        float time = GetTime();
        float res[2] = { (float)GetScreenWidth(), (float)GetScreenHeight() };

        SetShaderValue(shader, timeLoc, &time, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, resLoc, res, SHADER_UNIFORM_VEC2);

        float pos[3] = {camera.position.x, camera.position.y, camera.position.z};

        SetShaderValue(shader, flenLoc, &camera.fovy, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, camCenLoc, pos, SHADER_UNIFORM_VEC3);

        BeginDrawing();
            BeginShaderMode(shader);
                DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), WHITE);
            EndShaderMode();
            DrawFPS(5, 5);
            DrawInfo(camera);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
