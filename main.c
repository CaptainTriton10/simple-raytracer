#include "raylib.h"

int main(void) {
    // 1. Initialize Window
    const int screenWidth = 512;
    const int screenHeight = 512;
    InitWindow(screenWidth, screenHeight, "Simple Raytracer");

    SetTargetFPS(24);

    while (!WindowShouldClose()) {    // Detect window close button or ESC key
        Shader shader = LoadShader(0, "raytracing.frag");

        int timeLoc = GetShaderLocation(shader, "time");
        int resLoc  = GetShaderLocation(shader, "resolution");

        while (!WindowShouldClose())
        {
            float time = GetTime();
            float res[2] = { (float)GetScreenWidth(), (float)GetScreenHeight() };

            SetShaderValue(shader, timeLoc, &time, SHADER_UNIFORM_FLOAT);
            SetShaderValue(shader, resLoc, res, SHADER_UNIFORM_VEC2);

            BeginDrawing();
                BeginShaderMode(shader);
                    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), WHITE);
                EndShaderMode();
            EndDrawing();
        }
    }

    CloseWindow();

    return 0;
}
