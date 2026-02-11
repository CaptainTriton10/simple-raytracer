#include "../include/helpers.h"
#include "../include/bvh.h"
#include "raylib.h"
#include "../include/tomlc17.h"
#include <math.h>
#include <time.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_OBJECTS 13
#define DATA_WIDTH 5
#define BVH_DATA_WIDTH 3

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

Texture2D CreateSphereData(Sphere spheres[], size_t len) {
    size_t dataSize = len * DATA_WIDTH * 4;
    float *data = malloc(dataSize * sizeof(float));

    for (int i = 0; i < len; i++) {
        int base = i * DATA_WIDTH * 4;

        // (0, 0)
        data[base + 0] = SPHERE;
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

        // (4, 0)
        data[base + 16] = spheres[i].material.emission[0];
        data[base + 17] = spheres[i].material.emission[1];
        data[base + 18] = spheres[i].material.emission[2];
        data[base + 19] = 0.0f;
    }

    Image dataImage = {
        .data = data,
        .width = DATA_WIDTH,
        .height = len,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R32G32B32A32
    };

    Texture2D dataTexture = LoadTextureFromImage(dataImage);

    SetTextureFilter(dataTexture, TEXTURE_FILTER_POINT);
    SetTextureWrap(dataTexture, TEXTURE_WRAP_CLAMP);

    return dataTexture;
}

Texture2D CreateBVHData(BVHNode nodes[], size_t nodeCount) {
    size_t dataSize = nodeCount * BVH_DATA_WIDTH * 4;
    float *data = malloc(dataSize * sizeof(float));

    for (int i = 0; i < nodeCount; i++) {
        int base = i * BVH_DATA_WIDTH * 4;

        data[base + 0] = nodes[i].bbox.x.min;
        data[base + 1] = nodes[i].bbox.y.min;
        data[base + 2] = nodes[i].bbox.z.min;
        data[base + 3] = 0.0f;

        data[base + 4] = nodes[i].bbox.x.max;
        data[base + 5] = nodes[i].bbox.y.max;
        data[base + 6] = nodes[i].bbox.z.max;
        data[base + 7] = 0.0f;

        data[base + 8] = nodes[i].left.type;
        data[base + 9] = nodes[i].left.index;
        data[base + 10] = nodes[i].right.type;
        data[base + 11] = nodes[i].right.index;
    }

    Image dataImage = {
        .data = data,
        .width = BVH_DATA_WIDTH,
        .height = nodeCount,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R32G32B32A32
    };

    Texture2D dataTexture = LoadTextureFromImage(dataImage);

    SetTextureFilter(dataTexture, TEXTURE_FILTER_POINT);
    SetTextureWrap(dataTexture, TEXTURE_WRAP_CLAMP);

    return dataTexture;
}

RenderSettings ParseRendererConfig(const char *filename) {
    const char headerName[] = "settings";
    toml_result_t result = toml_parse_file_ex(filename);

    if (!result.ok) {
        char errMsg[128];
        sprintf(errMsg, "%s config parse error.", filename);

        error(errMsg);
    }

    toml_datum_t widthT = GetConfigParam(result, (char*)headerName, "width", TOML_INT64);
    if (widthT.u.int64 <= 0) {
        error("Width cannot be negative or 0.");
    }

    toml_datum_t heightT = GetConfigParam(result, (char*)headerName, "height", TOML_INT64);
    if (heightT.u.int64 <= 0) {
        error("Height cannot be negative or 0.");
    }

    toml_datum_t fullscreenT = GetConfigParam(result, (char*)headerName, "fullscreen", TOML_BOOLEAN);

    float fovLimits[2];
    GetConfigVec2(result, fovLimits, (char*)headerName, "fov_limits");
    if (fovLimits[0] > fovLimits[1]) {
        char errMsg[256];
        sprintf(errMsg, "The first argument of fov_limits cannot be greater than the second [%f > %f].", fovLimits[0], fovLimits[1]);

        error(errMsg);
    }


    toml_datum_t fovT = GetConfigParam(result, (char*)headerName, "fov", TOML_FP64);

    float camPos[3];
    GetConfigVec3(result, camPos, (char*)headerName, "camera_position");

    toml_datum_t aaT = GetConfigParam(result, (char*)headerName, "anti_aliasing", TOML_BOOLEAN);

    RenderSettings settings = {
        .width = widthT.u.int64,
        .height = heightT.u.int64,
        .fov = fovT.u.fp64,
        .fullscreen = fullscreenT.u.boolean,
        .aaEnabled = aaT.u.boolean
    };

    memcpy(settings.fovLimits, fovLimits, sizeof(fovLimits));
    memcpy(settings.cameraPosition, camPos, sizeof(camPos));

    return settings;
}

Scene ParseSceneConfig(const char *filename) {
    toml_result_t result = toml_parse_file_ex(filename);

    if (!result.ok) {
        error("Config parse error.");
    }

    // Get objects and materials
    toml_datum_t objectsT = GetConfigParam(result, "data", "objects", TOML_ARRAY);

    const size_t objCount = objectsT.u.arr.size;
    char *objNames[objCount];

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

    for (int i = 0; i < objCount; i++) {
        free(objNames[i]);
    }

    return scene;
}

void CreateBVH(Scene *scene) {
    printf("Creating BVH...\n");
    double startTime = GetTime();

    ComputeWorldBBoxes(scene);

    scene->nodes = malloc(sizeof(BVHNode) * (2 * scene->objCount));
    scene->nodeCount = 0;

    int root = InitBVHNode(scene, 0, scene->objCount);

    printf("BVH created: %d nodes\n", scene->nodeCount);
    double totalTime = (GetTime() - startTime) * 1000;
    printf("BVH creation time: %fms\n", totalTime);
}

int main(int argc, char *argv[]) {
    RenderSettings settings = ParseRendererConfig("./configs/renderer.toml");

    Scene scene;

    if (argc == 1) {
        scene = ParseSceneConfig("./configs/scene.toml");
    } else if (argc == 2) {
        scene = ParseSceneConfig(argv[1]);
    } else {
        fprintf(stderr, "Incorrect number of arguments (%d).", argc);
        exit(1);
    }

    CreateBVH(&scene);

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

    SetTargetFPS(100);

    Texture2D data = CreateSphereData(scene.objects, scene.objCount);
    Texture2D bvhData = CreateBVHData(scene.nodes, scene.nodeCount);

    Shader raytracing = LoadShader(0, "src/shaders/raytracing.frag");
    Shader denoiser = LoadShader(0, "src/shaders/denoise.frag");

    DenoiserShaderLocations denoiserLocs = GetDenoiserLocations(denoiser);
    RaytracerShaderLocations raytracerLocs = GetRaytracerLocations(raytracing);

    RenderTexture prevFrame = LoadRenderTexture(screenWidth, screenHeight);
    RenderTexture accA = LoadRenderTexture(screenWidth, screenHeight);
    RenderTexture accB = LoadRenderTexture(screenWidth, screenHeight);

    Image uvImg = LoadImage("uv.jpg");
    Texture2D uvTex = LoadTextureFromImage(uvImg);

    bool useA = true;

    float yaw = -90.0f * DEG2RAD;
    float pitch = 0.0f;

    DisableCursor();

    int frame = 0;
    while (!WindowShouldClose()) {    // Detect window close button or ESC key
        double startTime1 = GetTime();
        float res[2] = { (float)GetScreenWidth(), (float)GetScreenHeight() };
        float time_s = GetTime();

        BasisVectors vectors = Look(&yaw, &pitch);

        int changed = 0;
        if (Movement(&camera, vectors) || Zoom(&camera, settings) || Settings(&settings)) {
            changed = 1;
        } else if (GetMouseDelta().x != 0.0f || GetMouseDelta().y != 0.0f) {
            changed = 1;
        }

        float pos[3] = {camera.position.x, camera.position.y, camera.position.z};

        RaytracerShaderValues raytracerValues = {
            .time = time_s,
            .resolution = res,
            .dataSize = scene.objCount,
            .fov = camera.fovy,
            .cameraCenter = pos,
            .forward = vectors.forward,
            .right = vectors.right,
            .up = vectors.up,
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
        int bvhDataLoc = GetShaderLocation(raytracing, "bvhData");
        int uvTexLoc = GetShaderLocation(raytracing, "uvTex");

        BeginTextureMode(prevFrame);
            ClearBackground(BLACK);
            BeginShaderMode(raytracing);
                SetShaderValueTexture(raytracing, dataLoc, data);   // The data must be loaded here
                SetShaderValueTexture(raytracing, bvhDataLoc, bvhData);
                SetShaderValueTexture(raytracing, uvTexLoc, uvTex);
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
            EndDrawing();

            useA = !useA;
        }

        frame++;
        double totalTime = GetTime() - startTime1;
        printf("Total frame time: %fms [%.1ffps]\n\n", totalTime * 1000, 1.0f / totalTime);
    }

    CloseWindow();
    SceneFree(&scene);

    return 0;
}
