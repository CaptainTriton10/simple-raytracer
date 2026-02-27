#include "renderer.h"

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
