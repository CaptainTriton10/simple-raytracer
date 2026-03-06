#include "objects/objects.h"
#include "renderer/renderer.h"
#include "bvh/bvh.h"
#include "configs/configs.h"
#include "utils/utils.h"
#include "controls/controls.h"
#include "gui/gui.h"
#include <raylib.h>
#include <raymath.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_OBJECTS 13

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

/*
 * Sphere Data Packing:
 * Sphere 1 - width = 4
 *      (0, 0):
 *          r = type
 *      (1, 0):
 *          rgb = position
 *          a = radius
 *      (2, 0):
 *          r = scatter type
 *          gba = albedo
 *      (3, 0):
 *          r = roughness
 *          g = ior
 *      (4, 0):
 *          rgb = emission
 */

int main(int argc, char *argv[]) {
    RenderSettings settings = ParseRendererConfig("./configs/renderer.toml");

    Atlas atlas = GetTextures(settings.texturesPath, settings.atlasChunkSize);
    Image atlasImage = CreateAtlas(atlas);

    Scene scene;

    if (argc == 1) {
        scene = ParseSceneConfig("./configs/scene.toml", atlas);
    } else if (argc == 2) {
        scene = ParseSceneConfig(argv[1], atlas);
    } else {
        fprintf(stderr, "Incorrect number of arguments (%d).", argc);
        exit(1);
    }

    BVHNode *bvhNodes = malloc(sizeof(BVHNode) * (2 * scene.objCount));
    CreateBVH(&scene, bvhNodes);

    const int screenWidth = settings.width;
    const int screenHeight = settings.height;

    if (settings.fullscreen == 1) SetConfigFlags(FLAG_FULLSCREEN_MODE);

    InitWindow(screenWidth, screenHeight, "Simple Raytracer");

    Camera camera = {
        .fovy = settings.fov
    };

    camera.position.x = settings.cameraPosition[0];
    camera.position.y = settings.cameraPosition[1];
    camera.position.z = settings.cameraPosition[2];

    SetTargetFPS(1000);

    Texture2D data = CreateSceneData(scene.objects, scene.objCount);
    Texture2D bvhData = CreateBVHData(bvhNodes, scene.nodeCount);

    // Shader raytracing = InjectShaderData("src/shaders/raytracing.frag", scene);
    Shader raytracing = LoadShader(0, "src/shaders/raytracing.frag");
    Shader denoiser = LoadShader(0, "src/shaders/denoise.frag");

    DenoiserShaderLocations denoiserLocs = GetDenoiserLocations(denoiser);
    RaytracerShaderLocations raytracerLocs = GetRaytracerLocations(raytracing);

    RenderTexture prevFrame = LoadRenderTexture(screenWidth, screenHeight);
    RenderTexture accA = LoadRenderTexture(screenWidth, screenHeight);
    RenderTexture accB = LoadRenderTexture(screenWidth, screenHeight);

    Texture2D atlasTexture = LoadTextureFromImage(atlasImage);

    bool useA = true;
    bool cursorEnabled = false;
    DisableCursor();

    GUI gui = {
        .sidebar = {
            .selected = 0,
            .properties = {
                .position = {
                    {
                        {
                            .value = 1.0,
                            .textVal = "1.0",
                            .editMode = false
                        }, {
                            .value = 1.0,
                            .textVal = "1.0",
                            .editMode = false
                        }, {
                            .value = 1.0,
                            .textVal = "1.0",
                            .editMode = false
                        }
                    }
                },
                .scroll = Vector2Zero()
            },
            .outliner = {
                .scroll = Vector2Zero(),
                .objects = {
                    {
                        .name = "Object 1",
                        .index = 0
                    },
                    {
                        .name = "Object 2",
                        .index = 1
                    }
                }
            }
        }
    };

    float yaw = -90.0f * DEG2RAD;
    float pitch = 0.0f;

    int frame = 0;
    while (!WindowShouldClose()) {    // Detect window close button or ESC key
        double startTime1 = GetTime();
        float res[2] = { (float)GetScreenWidth(), (float)GetScreenHeight() };
        float time_s = GetTime();

        BasisVectors vectors = Look(&yaw, &pitch, cursorEnabled);

        int changed = 1;
        if (Movement(&camera, vectors) || Zoom(&camera, settings) || Settings(&settings)) {
            changed = 1;
        } else if ((GetMouseDelta().x != 0.0f || GetMouseDelta().y != 0.0f) && !cursorEnabled) {
            changed = 1;
        }

        ToggleCursor(&cursorEnabled);

        float pos[3] = {camera.position.x, camera.position.y, camera.position.z};

        RaytracerShaderValues raytracerValues = {
            .time = time_s,
            .resolution = res,
            .dataSize = scene.objCount,
            .fov = camera.fovy,
            .cameraCenter = pos,
            .skyColour = scene.sky,
            .forward = vectors.forward,
            .right = vectors.right,
            .up = vectors.up,
            .antiAliasing = settings.aaEnabled,
            .maxDepth = settings.maxDepth,
            .chunkSize = settings.atlasChunkSize
        };

        if (changed == 1) {
            ClearTexture(accA);
            ClearTexture(accB);

            frame = 0;
            useA = true;
        }

        SetRaytracerValues(raytracing, raytracerLocs, raytracerValues);

        int dataLoc = GetShaderLocation(raytracing, "data");
        int bvhDataLoc = GetShaderLocation(raytracing, "bvhData");
        int atlasLoc = GetShaderLocation(raytracing, "textureAtlas");

        BeginTextureMode(prevFrame);
            ClearBackground(BLACK);
            BeginShaderMode(raytracing);
                SetShaderValueTexture(raytracing, dataLoc, data);   // The data must be loaded here
                SetShaderValueTexture(raytracing, bvhDataLoc, bvhData);
                SetShaderValueTexture(raytracing, atlasLoc, atlasTexture);
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
                DrawInfo(camera, settings, frame, (Vector2){yaw, pitch});
                DrawCircle(res[0] / 2.0, res[1] / 2.0, 2.0, BLACK);

                MainGUI(&settings, &gui, &scene);
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
                DrawInfo(camera, settings, frame, (Vector2){yaw, pitch});
                DrawCircle(res[0] / 2.0, res[1] / 2.0, 2.0, BLACK);

                MainGUI(&settings, &gui, &scene);
            EndDrawing();

            useA = !useA;
        }

        frame++;
        double totalTime = GetTime() - startTime1;
        // printf("Total frame time: %fms [%.1ffps]\n\n", totalTime * 1000, 1.0f / totalTime);
    }

    CloseWindow();
    SceneFree(&scene);

    return EXIT_SUCCESS;
}
