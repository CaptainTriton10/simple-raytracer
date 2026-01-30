#include "../include/helpers.h"
#include "raylib.h"
#include "../include/tomlc17.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_OBJECTS 4
#define DATA_WIDTH 4

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
 */

Texture2D CreateSphereData(Sphere spheres[], size_t len) {
    size_t dataSize = len * DATA_WIDTH * 4;
    float *data = malloc(dataSize * sizeof(float));

    for (int i = 0; i < len; i++) {
        int base = i * DATA_WIDTH * 4;

        // (0, 0)
        data[base + 0] = 0; // Sphere type
        data[base + 1] = 0.0f; // Empty (unused)
        data[base + 2] = 0.0f;
        data[base + 3] = 0.0f;

        // (1, 0)
        data[base + 4] = spheres[i].pos[0];
        data[base + 5] = spheres[i].pos[1];
        data[base + 6] = spheres[i].pos[2];
        data[base + 7] = spheres[i].radius;

        // (2, 0)
        data[base + 8] = spheres[i].material.type;
        data[base + 9] = spheres[i].material.albedo[0];
        data[base + 10] = spheres[i].material.albedo[1];
        data[base + 11] = spheres[i].material.albedo[2];

        // (3, 0)
        data[base + 12] = spheres[i].material.roughness;
        data[base + 13] = spheres[i].material.ior;
        data[base + 14] = 0.0f;
        data[base + 15] = 0.0f;
    }

    Image dataImage = {
        .data = data,
        .width = DATA_WIDTH,
        .height = len,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R32G32B32A32
    };

    ExportImage(dataImage, "asdf.png");

    Texture2D dataTexture = LoadTextureFromImage(dataImage);

    SetTextureFilter(dataTexture, TEXTURE_FILTER_POINT);
    SetTextureWrap(dataTexture, TEXTURE_WRAP_CLAMP);

    return dataTexture;
}

Scene ParseSceneConfig(const char *filename) {
    toml_result_t result = toml_parse_file_ex(filename);

    if (!result.ok) {
        error("Config parse error.");
    }

    // Get objects and materials
    toml_datum_t objectsT = GetConfigParam(result, "data", "objects", TOML_ARRAY);
    toml_datum_t materialsT = GetConfigParam(result, "data", "materials", TOML_ARRAY);

    const size_t matCount = materialsT.u.arr.size;
    char *matNames[matCount];

    const size_t objCount = objectsT.u.arr.size;
    char *objNames[objCount];

    // Get the names of all the materials
    for (int i = 0; i < matCount; i++) {    // Loop over each name
        if (materialsT.u.arr.elem[i].type == TOML_STRING){
            matNames[i] = _strdup(materialsT.u.arr.elem[i].u.s);
        } else {
            error("Material name is not a string.");
        }
    }

    Sphere *objects = malloc(objCount * sizeof(Sphere));

    // Get the names of all the objects
    for (int i = 0; i < objCount; i++) {
        if (objectsT.u.arr.elem[i].type == TOML_STRING){
            objNames[i] = _strdup(objectsT.u.arr.elem[i].u.s);

            objects[i] = GetObjectParams(result, objNames[i]);
        } else {
            error("Object name is not a string.");
        }
    }

    Scene scene = {
        .objCount = objCount,
        .objects = objects
    };

    toml_free(result);

    for (int i = 0; i < matCount; i++) {
        free(matNames[i]);
    }

    for (int i = 0; i < objCount; i++) {
        free(objNames[i]);
    }

    return scene;
}

int main() {
    Scene scene = ParseSceneConfig("./configs/test.toml");

    RenderSettings settings = {
        .aaEnabled = 0,
        .width = 1920
    };

    const float aspectRatio = 16.0f / 9.0f;

    const int screenWidth = settings.width;
    const int screenHeight = (int)(screenWidth / aspectRatio);

    SetConfigFlags(FLAG_FULLSCREEN_MODE);

    InitWindow(screenWidth, screenHeight, "Simple Raytracer");

    Camera camera = {
        .position = {0.0f, 0.0f, 2.0f},
        .fovy = 2.0f
    };

    SetTargetFPS(100);

    Texture2D data = CreateSphereData(scene.objects, scene.objCount);

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
        if (Movement(&camera) || Zoom(&camera) || Settings(&settings)) {
            changed = 1;
        }

        float pos[3] = {camera.position.x, camera.position.y, camera.position.z};

        RaytracerShaderValues raytracerValues = {
            .time = time,
            .resolution = res,
            .dataSize = scene.objCount,
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

        SetRaytracerValues(raytracing, raytracerLocs, raytracerValues);

        int dataLoc = GetShaderLocation(raytracing, "data");

        BeginTextureMode(prevFrame);
            ClearBackground(BLACK);
            BeginShaderMode(raytracing);
                SetShaderValueTexture(raytracing, dataLoc, data);   // The data must be loaded here
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
    SceneFree(&scene);

    return 0;
}
