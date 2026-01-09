#include "raylib.h"

#define CAMERA_MOVE_SPEED 1.5

void Movement(float cameraCenter[3]) {
    float move = CAMERA_MOVE_SPEED * GetFrameTime();

    if (IsKeyDown(KEY_W)) {
        cameraCenter[2] += -move;
    }

    if (IsKeyDown(KEY_A)) {
        cameraCenter[0] += -move;
    }

    if (IsKeyDown(KEY_S)) {
        cameraCenter[2] += move;
    }

    if (IsKeyDown(KEY_D)) {
        cameraCenter[0] += move;
    }

    if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_C)) {
        cameraCenter[1] += -move;
    }

    if (IsKeyDown(KEY_SPACE)) {
        cameraCenter[1] += move;
    }
}

int main(void) {
    const float aspectRatio = 2.0f / 1.0f;

    const int screenWidth = 1280;
    const int screenHeight = (int)(screenWidth / aspectRatio);

    InitWindow(screenWidth, screenHeight, "Simple Raytracer");

    float cameraCenter[3] = {0.0f, 0.0f, 0.0f};
    SetTargetFPS(100);

    Shader shader = LoadShader(0, "raytracing.frag");

    int timeLoc = GetShaderLocation(shader, "time");
    int resLoc  = GetShaderLocation(shader, "resolution");

    int flenLoc = GetShaderLocation(shader, "focalLength");
    int camCenLoc = GetShaderLocation(shader, "cameraCenter");
    int viewpLoc = GetShaderLocation(shader, "viewport");

    while (!WindowShouldClose()) {    // Detect window close button or ESC key

        float viewportHeight = 2.0f;

        float focalLength = 2.0f;

        Movement(cameraCenter);

        float time = GetTime();
        float res[2] = { (float)GetScreenWidth(), (float)GetScreenHeight() };

        float viewport[2] = {viewportHeight, viewportHeight*((float)screenWidth / (float)screenHeight)};

        SetShaderValue(shader, timeLoc, &time, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, resLoc, res, SHADER_UNIFORM_VEC2);

        SetShaderValue(shader, flenLoc, &focalLength, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, camCenLoc, cameraCenter, SHADER_UNIFORM_VEC3);
        SetShaderValue(shader, viewpLoc, viewport, SHADER_UNIFORM_VEC3);

        BeginDrawing();
            BeginShaderMode(shader);
                DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), WHITE);
            EndShaderMode();
            DrawFPS(5, 5);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
