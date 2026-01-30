#include "../include/helpers.h"
#include "../include/tomlc17.h"
#include "raylib.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define CAMERA_MOVE_SPEED 1.5
#define CAMERA_ZOOM_SPEED 4

void error(const char *msg) {
    fprintf(stderr, "ERROR: %s\n", msg);
    exit(1);
}

toml_datum_t GetConfigParam(toml_result_t table, char *section, char *item, toml_type_t type) {
    char path[64];
    sprintf(path, "%s.%s", section, item);

    toml_datum_t param = toml_seek(table.toptab, path);
    if (param.type != type) {
        char errMsg[128];
        sprintf(errMsg, "Missing or invalid %s property", path);

        error(errMsg);
    }

    return param;
}

void GetConfigVec3(toml_result_t table, float *vec, char *section, char *item) {
    char path[64];
    sprintf(path, "%s.%s", section, item);

    toml_datum_t param = toml_seek(table.toptab, path);
    if (param.type != TOML_ARRAY) {
        char errMsg[128];
        sprintf(errMsg, "Missing or invalid %s property", path);

        error(errMsg);
    } else if(param.u.arr.size != 3) {
        char errMsg[128];
        sprintf(errMsg, "Wrong number of arguments (%d) for vec3 [%s]", param.u.arr.size, path);

        error(errMsg);
    }

    float result[3];

    for (int i = 0; i < 3; i++) {
        result[i] = (float) param.u.arr.elem[i].u.fp64;
    }

    memcpy(vec, result, sizeof(result));    // Move result to input float array
}

Sphere GetObjectParams(toml_result_t table, char *name) {
    float position[3];
    GetConfigVec3(table, position, name, "position");

    toml_datum_t radiusT = GetConfigParam(table, name, "radius", TOML_FP64);
    toml_datum_t materialT = GetConfigParam(table, name, "material", TOML_STRING);
    char *matName = _strdup(materialT.u.s);

    toml_datum_t typeT = GetConfigParam(table, matName, "type", TOML_INT64);

    float albedo[3];
    GetConfigVec3(table, albedo, matName, "albedo");

    toml_datum_t roughnessT = GetConfigParam(table, matName, "roughness", TOML_FP64);
    toml_datum_t iorT = GetConfigParam(table, matName, "ior", TOML_FP64);

    ShaderMaterial material = {
        .type = typeT.u.int64,
        .roughness = roughnessT.u.fp64,
        .ior = iorT.u.fp64
    };

    memcpy(material.albedo, albedo, sizeof(albedo));

    Sphere obj = {
        .radius = radiusT.u.fp64,
        .material = material
    };

    memcpy(obj.pos, position, sizeof(position));

    free(matName);
    return obj;
}

void SceneFree(Scene *scene) {
    if (!scene) return;

    free(scene->objects);
}

RaytracerShaderLocations GetRaytracerLocations(Shader shader) {
    RaytracerShaderLocations locs = {
        .time = GetShaderLocation(shader, "time"),
        .resolution = GetShaderLocation(shader, "resolution"),
        .focalLength = GetShaderLocation(shader, "focalLength"),
        .cameraCenter = GetShaderLocation(shader, "cameraCenter"),
        .antiAliasing = GetShaderLocation(shader, "aaEnabled"),
        .dataSize = GetShaderLocation(shader, "dataSize")
    };

    return locs;
}

void SetRaytracerValues(Shader shader, RaytracerShaderLocations locs, RaytracerShaderValues values) {
    SetShaderValue(shader, locs.time, &values.time, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, locs.resolution, values.resolution, SHADER_UNIFORM_VEC2);

    SetShaderValue(shader, locs.dataSize, &values.dataSize, SHADER_UNIFORM_INT);

    SetShaderValue(shader, locs.focalLength, &values.focalLength, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, locs.cameraCenter, values.cameraCenter, SHADER_UNIFORM_VEC3);

    SetShaderValue(shader, locs.antiAliasing, &values.antiAliasing, SHADER_UNIFORM_INT);
}

DenoiserShaderLocations GetDenoiserLocations(Shader shader) {
    DenoiserShaderLocations locs = {
        .resolution = GetShaderLocation(shader, "resolution"),
        .prevFrame = GetShaderLocation(shader, "prevFrame"),
        .accRender = GetShaderLocation(shader, "accRender"),
        .changed = GetShaderLocation(shader, "changed"),
        .frame = GetShaderLocation(shader, "frame")
    };

    return locs;
}

void SetDenoiserValues(Shader shader, DenoiserShaderLocations locs, DenoiserShaderValues values) {
    SetShaderValue(shader, locs.resolution, values.resolution, SHADER_UNIFORM_VEC2);
    SetShaderValue(shader, locs.changed, &values.changed, SHADER_UNIFORM_INT);
    SetShaderValue(shader, locs.frame, &values.frame, SHADER_UNIFORM_INT);
}

float Clampf(float value, float min, float max) {
    return fmaxf(min, fminf(value, max));
}

bool Movement(Camera *camera) {
    float move = CAMERA_MOVE_SPEED * GetFrameTime();
    bool changed = false;

    if (IsKeyDown(KEY_W)) {
        camera->position.z += -move;
        changed = true;
    }

    if (IsKeyDown(KEY_A)) {
        camera->position.x += -move;
        changed = true;
    }

    if (IsKeyDown(KEY_S)) {
        camera->position.z += move;
        changed = true;
    }

    if (IsKeyDown(KEY_D)) {
        camera->position.x += move;
        changed = true;
    }

    if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_C)) {
        camera->position.y += -move;
        changed = true;
    }

    if (IsKeyDown(KEY_SPACE)) {
        camera->position.y += move;
        changed = true;
    }

    return changed;
}

bool Zoom(Camera *camera) {
    float zoomFactor = CAMERA_ZOOM_SPEED * GetFrameTime();
    float scroll = 1 + zoomFactor * GetMouseWheelMove();

    camera->fovy = Clampf(camera->fovy * scroll, 0.1, 30);

    // If the camera was zoomed this frame
    if (scroll != 1.0) {
        return true;
    }

    return false;
}

bool Settings(RenderSettings *settings) {
    if (IsKeyPressed(KEY_ONE)) {
        settings->aaEnabled = settings->aaEnabled == 1 ? 0 : 1;
        return true;
    }

    return false;
}

void DrawInfo(Camera camera, RenderSettings settings, int frame) {
    char frameInfo[16];
    sprintf(frameInfo, "Frame: %d", frame);

    char cameraPosInfo[128];
    sprintf(cameraPosInfo, "Camera Position: [%.2f, %.2f, %.2f]", camera.position.x, camera.position.y, camera.position.z);

    char cameraFovyInfo[64];
    sprintf(cameraFovyInfo, "Camera Focal Length: %.2f", camera.fovy);

    char aaInfo[64];
    sprintf(aaInfo, "Anti-Aliasing: %d", settings.aaEnabled);

    DrawFPS(5, 5);

    DrawText(cameraPosInfo, 5, 50, 20, RED);
    DrawText(cameraFovyInfo, 5, 75, 20, RED);

    DrawText(aaInfo, 5, 125, 20, YELLOW);

    DrawText(frameInfo, 5, 175, 20, PURPLE);
}

void CopyTexture(RenderTexture source, RenderTexture target, float resolution[2]) {
    BeginTextureMode(target);
        DrawTextureRec(
            source.texture,
            (Rectangle){ 0, 0, (float)resolution[0], -(float)resolution[1] },
            (Vector2){ 0, 0 },
            WHITE
        );
    EndTextureMode();
}

void ClearTexture(RenderTexture tex) {
    BeginTextureMode(tex);
        ClearBackground(BLACK);
    EndTextureMode();
}
