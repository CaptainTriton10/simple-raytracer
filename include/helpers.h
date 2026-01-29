#ifndef HELPERS_H
#define HELPERS_H

#include "raylib.h"
#include <stddef.h>

typedef struct ShaderMaterial {
    int type;
    float *albedo;
    float roughness;
    float ior;
} ShaderMaterial;

typedef struct Sphere {
    float *pos;
    float radius;
    ShaderMaterial material;
} Sphere;

typedef struct RenderSettings {
    int aaEnabled;
    int width;
    int height;
} RenderSettings;

typedef struct RaytracerShaderValues {
    float time;
    float *resolution;
    float focalLength;
    float *cameraCenter;
    int antiAliasing;
    int dataSize;
} RaytracerShaderValues;

typedef struct RaytracerShaderLocations {
    int time;
    int resolution;
    int focalLength;
    int cameraCenter;
    int antiAliasing;
    int dataSize;
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

float Clampf(float value, float min, float max);

bool Movement(Camera *camera);
bool Zoom(Camera *camera);

bool Settings(RenderSettings *settings);
void DrawInfo(Camera camera, RenderSettings settings, int frame);

void CopyTexture(RenderTexture source, RenderTexture target, float resolution[2]);
void ClearTexture(RenderTexture tex);

#endif
