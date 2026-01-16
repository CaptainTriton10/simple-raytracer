#include "../include/helpers.h"
#include "raylib.h"
#include <math.h>

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

int main(void) {
    const float aspectRatio = 2.0f / 1.0f;

    const int screenWidth = 1280;
    const int screenHeight = (int)(screenWidth / aspectRatio);

    InitWindow(screenWidth, screenHeight, "Simple Raytracer");

    Camera camera = {
        .position = {0.0f, 0.0f, 2.0f},
        .fovy = 2.0f
    };

    RenderSettings settings = {
        .aaEnabled = false
    };

    SetTargetFPS(100);

    Shader raytracing = LoadShader(0, "src/shaders/raytracing.frag");
    Shader denoiser = LoadShader(0, "src/shaders/denoise.frag");

    int resLocDns = GetShaderLocation(denoiser, "resolution");

    int prevFrLoc = GetShaderLocation(denoiser, "prevFrame");
    int accRndLoc = GetShaderLocation(denoiser, "accRender");

    int changedLoc = GetShaderLocation(denoiser, "changed");

    int timeLoc = GetShaderLocation(raytracing, "time");
    int resLocRTX  = GetShaderLocation(raytracing, "resolution");

    int flenLoc = GetShaderLocation(raytracing, "focalLength");
    int camCenLoc = GetShaderLocation(raytracing, "cameraCenter");
    int viewpLoc = GetShaderLocation(raytracing, "viewport");

    int aaLoc = GetShaderLocation(raytracing, "aaEnabled");

    RenderTexture prevFrame = LoadRenderTexture(screenWidth, screenHeight);
    RenderTexture accA = LoadRenderTexture(screenWidth, screenHeight);
    RenderTexture accB = LoadRenderTexture(screenWidth, screenHeight);

    bool useA = true;

    int frame = 0;
    while (!WindowShouldClose()) {    // Detect window close button or ESC key
        float pos[3] = {camera.position.x, camera.position.y, camera.position.z};
        float res[2] = { (float)GetScreenWidth(), (float)GetScreenHeight() };
        float time = GetTime();

        int changed = 0;

        changed = Movement(&camera) ? 1 : 0;
        Zoom(&camera);
        Settings(&settings);

        // Boring stuff
        SetShaderValue(raytracing, timeLoc, &time, SHADER_UNIFORM_FLOAT);
        SetShaderValue(raytracing, resLocRTX, res, SHADER_UNIFORM_VEC2);

        // Fun stuff :) (camera settings)
        SetShaderValue(raytracing, flenLoc, &camera.fovy, SHADER_UNIFORM_FLOAT);
        SetShaderValue(raytracing, camCenLoc, pos, SHADER_UNIFORM_VEC3);

        // Render settings
        SetShaderValue(raytracing, aaLoc, &settings.aaEnabled, SHADER_UNIFORM_INT);

        BeginTextureMode(prevFrame);
            ClearBackground(BLACK);
            BeginShaderMode(raytracing);
                DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), WHITE);
            EndShaderMode();
        EndTextureMode();

        if (frame == 0) {
            CopyTexture(prevFrame, accA, res);

            BeginDrawing();
                ClearBackground(WHITE);
                DrawTextureRec(
                    accA.texture,
                    (Rectangle){ 0, 0, (float)screenWidth, -(float)screenHeight },
                    (Vector2){ 0, 0 },
                    WHITE
                );
                DrawInfo(camera, settings, frame);
            EndDrawing();
        } else {
            SetShaderValue(denoiser, resLocDns, res, SHADER_UNIFORM_VEC2);
            SetShaderValue(denoiser, changedLoc, &changed, SHADER_UNIFORM_INT);

            BeginTextureMode(useA ? accB : accA);
                ClearBackground(BLACK);
                BeginShaderMode(denoiser);
                    SetShaderValueTexture(denoiser, prevFrLoc, prevFrame.texture);
                    SetShaderValueTexture(denoiser, accRndLoc, useA ? accA.texture : accB.texture);

                    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), WHITE);
                EndShaderMode();
            EndTextureMode();

            BeginDrawing();
                ClearBackground(WHITE);
                DrawTextureRec(
                    useA ? accB.texture : accA.texture,
                    (Rectangle){ 0, 0, (float)screenWidth, -(float)screenHeight },
                    (Vector2){ 0, 0 },
                    WHITE
                );
                DrawInfo(camera, settings, frame);
            EndDrawing();

            useA = !useA;
        }

        frame++;
    }

    CloseWindow();

    return 0;
}








// BeginTextureMode(target);
//     BeginShaderMode(denoiser);
//         DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), WHITE);
//     EndShaderMode();
// EndTextureMode();

// BeginDrawing();
//     ClearBackground(MAGENTA);
//     DrawTextureRec(
//         target.texture,
//         (Rectangle){ 0, 0, (float)screenWidth, -(float)screenHeight },
//         (Vector2){ 0, 0 },
//         WHITE
//     );
//     DrawInfo(camera, settings, frame);
// EndDrawing();
