#ifndef RENDERER_H
#define RENDERER_H

#include <raylib.h>
#include "../objects/objects.h"

typedef struct RenderSettings {
    int width;
    int height;
    int fullscreen;
    float fovLimits[2];
    float fov;
    float cameraPosition[3];
    int aaEnabled;
    unsigned int maxDepth;
    char *texturesPath;
    int atlasChunkSize;
} RenderSettings;

typedef struct RaytracerShaderValues {
    float time;
    float *resolution;
    float fov;
    float *cameraCenter;
    float *skyColour;
    float *forward;
    float *right;
    float *up;
    int antiAliasing;
    int maxDepth;
    int dataSize;
    int chunkSize;
} RaytracerShaderValues;

typedef struct RaytracerShaderLocations {
    int time;
    int resolution;
    int fov;
    int cameraCenter;
    int skyColour;
    int forward;
    int right;
    int up;
    int antiAliasing;
    int maxDepth;
    int dataSize;
    int chunkSize;
} RaytracerShaderLocations;

typedef struct DenoiserShaderValues {
    float *resolution;
    int changed;
    int frame;
} DenoiserShaderValues;

typedef struct DenoiserShaderLocations {
    int resolution;
    int prevFrame;
    int accRender;
    int changed;
    int frame;
} DenoiserShaderLocations;

RaytracerShaderLocations GetRaytracerLocations(Shader shader);
void SetRaytracerValues(Shader shader, RaytracerShaderLocations locs, RaytracerShaderValues values);

DenoiserShaderLocations GetDenoiserLocations(Shader shader);
void SetDenoiserValues(Shader shader, DenoiserShaderLocations locs, DenoiserShaderValues values);

Shader InjectShaderData(const char *filename, Scene scene);

#endif
