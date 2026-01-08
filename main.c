#include "raylib.h"
#include <stdio.h>

// void CalculatePixelZero(float outArr[3], float resolution[2]) {
//     Vector3 cameraCenter = {0.0f, 0.0f, 0.0f};

//     float viewportheight = 2.0;
//     float viewportWidth = viewportheight * (resolution[0] / resolution[1]);

//     Vector3 viewportU = {viewportWidth, 0.0f, 0.0f};
//     Vector3 viewportV = {0.0, -viewportheight, 0.0f};

//     Vector3 pixelDeltaU = viewportU / resolution[0];
//     Vector3 pixelDeltaV = viewportV / resolution[1];
// }

int main(void) {
    // 1. Initialize Window

    const float aspectRatio = 2.0f / 1.0f;

    const int screenWidth = 1920;
    const int screenHeight = (int)(screenWidth / aspectRatio);

    InitWindow(screenWidth, screenHeight, "Simple Raytracer");

    float cameraCenter[3] = {0.0f, 0.0f, 0.0f};
    SetTargetFPS(60);

    while (!WindowShouldClose()) {    // Detect window close button or ESC key
        Shader shader = LoadShader(0, "raytracing.frag");

        int timeLoc = GetShaderLocation(shader, "time");
        int resLoc  = GetShaderLocation(shader, "resolution");

        int flenLoc = GetShaderLocation(shader, "focalLength");
        int camCenLoc = GetShaderLocation(shader, "cameraCenter");
        int viewpLoc = GetShaderLocation(shader, "viewport");

        float viewportHeight = 2.0f;

        float focalLength = 2.0f;

        if (IsKeyDown(KEY_W)) {
            cameraCenter[2] += -0.05f;
        }

        if (IsKeyDown(KEY_A)) {
            cameraCenter[0] += -0.05f;
        }

        if (IsKeyDown(KEY_S)) {
            cameraCenter[2] += 0.05f;
        }

        if (IsKeyDown(KEY_D)) {
            cameraCenter[0] += 0.05f;
        }

        if (IsKeyDown(KEY_SPACE)) {
            cameraCenter[1] += 0.05f;
        }

        if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_C)) {
            cameraCenter[1] += -0.05f;
        }

        printf("%f", cameraCenter[2]);


        float time = GetTime();
        float res[2] = { (float)GetScreenWidth(), (float)GetScreenHeight() };

        float viewport[2] = {viewportHeight, viewportHeight*((float)screenWidth / (float)screenHeight)};

        SetShaderValue(shader, timeLoc, &time, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, resLoc, res, SHADER_UNIFORM_VEC2);

        SetShaderValue(shader, flenLoc, &focalLength, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, camCenLoc, cameraCenter, SHADER_UNIFORM_VEC3);
        SetShaderValue(shader, viewpLoc, viewport, SHADER_UNIFORM_VEC3);

        struct Color textColour = {0, 0, 0, 255};


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
