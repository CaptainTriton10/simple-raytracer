#include "renderer.h"
#include "../utils/utils.h"
#include <stdio.h>

RaytracerShaderLocations GetRaytracerLocations(Shader shader) {
    RaytracerShaderLocations locs = {
        .time = GetShaderLocation(shader, "time"),
        .resolution = GetShaderLocation(shader, "resolution"),
        .fov = GetShaderLocation(shader, "fov"),
        .cameraCenter = GetShaderLocation(shader, "cameraCenter"),
        .skyColour = GetShaderLocation(shader, "skyColour"),
        .forward = GetShaderLocation(shader, "forward"),
        .right = GetShaderLocation(shader, "right"),
        .up = GetShaderLocation(shader, "up"),
        .antiAliasing = GetShaderLocation(shader, "aaEnabled"),
        .maxDepth = GetShaderLocation(shader, "maxDepth"),
        .dataSize = GetShaderLocation(shader, "dataSize"),
        .chunkSize = GetShaderLocation(shader, "chunkSize")
    };

    return locs;
}

void SetRaytracerValues(Shader shader, RaytracerShaderLocations locs, RaytracerShaderValues values) {
    SetShaderValue(shader, locs.time, &values.time, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, locs.resolution, values.resolution, SHADER_UNIFORM_VEC2);

    SetShaderValue(shader, locs.dataSize, &values.dataSize, SHADER_UNIFORM_INT);

    SetShaderValue(shader, locs.fov, &values.fov, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, locs.cameraCenter, values.cameraCenter, SHADER_UNIFORM_VEC3);

    SetShaderValue(shader, locs.skyColour, values.skyColour, SHADER_UNIFORM_VEC3);

    SetShaderValue(shader, locs.forward, values.forward, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, locs.right, values.right, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, locs.up, values.up, SHADER_UNIFORM_VEC3);

    SetShaderValue(shader, locs.antiAliasing, &values.antiAliasing, SHADER_UNIFORM_INT);
    SetShaderValue(shader, locs.maxDepth, &values.maxDepth, SHADER_UNIFORM_INT);
    SetShaderValue(shader, locs.chunkSize, &values.chunkSize, SHADER_UNIFORM_INT);
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

Shader InjectShaderData(const char *filename, Scene scene) {
    FILE *fptr;
    fopen_s(&fptr, filename, "r");

    if (fptr == NULL) {
        char errMsg[256];
        sprintf(errMsg, "Unable to open file [%s]", filename);

        error(errMsg);
    }

    // Get file size
    fseek(fptr, 0, SEEK_END);
    const size_t fileSize = ftell(fptr);
    fseek(fptr, 0, SEEK_SET);

    char shaderBuffer[fileSize];
    fread(shaderBuffer, sizeof(char), fileSize, fptr);

    char maxObj[16];
    sprintf(maxObj, "%lld", scene.objCount);
    char *temp1 = ReplaceSubstr(shaderBuffer, "${MAX_OBJ}", maxObj);

    char bvhStack[16];
    sprintf(bvhStack, "%d", scene.nodeCount);
    char *temp2 = ReplaceSubstr(temp1, "${MAX_BVH}", bvhStack);

    Shader shader = LoadShaderFromMemory(0, temp2);
    return shader;
}