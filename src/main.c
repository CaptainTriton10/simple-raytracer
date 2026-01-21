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
    const float aspectRatio = 16.0f / 9.0f;

    const int screenWidth = 1280;
    const int screenHeight = (int)(screenWidth / aspectRatio);

    SetConfigFlags(FLAG_FULLSCREEN_MODE);

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

    DenoiserShaderLocations denoiserLocs = GetDenoiserLocations(denoiser);
    RaytracerShaderLocations raytracerLocs = GetRaytracerLocations(raytracing);

    RenderTexture prevFrame = LoadRenderTexture(screenWidth, screenHeight);
    RenderTexture accA = LoadRenderTexture(screenWidth, screenHeight);
    RenderTexture accB = LoadRenderTexture(screenWidth, screenHeight);

    bool useA = true;

    int frame = 0;
    while (!WindowShouldClose()) {    // Detect window close button or ESC key
        float res[2] = { (float)GetScreenWidth(), (float)GetScreenHeight() };
        float time = GetTime();

        int changed = 0;
        changed = Movement(&camera) ? 1 : 0;

        float pos[3] = {camera.position.x, camera.position.y, camera.position.z};

        RaytracerShaderValues raytracerValues = {
            .time = time,
            .resolution = res,
            .focalLength = camera.fovy,
            .cameraCenter = pos,
            .antiAliasing = settings.aaEnabled
        };

        if (changed == 1) {
            ClearTexture(accA);
            ClearTexture(accB);

            frame = 0;
            useA = true;
        }

        Zoom(&camera);
        Settings(&settings);

        SetRaytracerValues(raytracing, raytracerLocs, raytracerValues);

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
            DenoiserShaderValues denoiserValues = {
                .resolution = res,
                .changed = changed,
                .frame = frame
            };

            SetDenoiserValues(denoiser, denoiserLocs, denoiserValues);

            BeginTextureMode(useA ? accB : accA);
                ClearBackground(BLACK);
                BeginShaderMode(denoiser);
                    SetShaderValueTexture(denoiser, denoiserLocs.prevFrame, prevFrame.texture);
                    SetShaderValueTexture(denoiser, denoiserLocs.accRender, useA ? accA.texture : accB.texture);

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
