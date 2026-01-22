#ifndef HELPERS_H
#define HELPERS_H

#include "raylib.h"

typedef struct RenderSettings {
    int aaEnabled;
} RenderSettings;

typedef struct RaytracerShaderValues {
    float time;
    float *resolution;
    float focalLength;
    float *cameraCenter;
    int antiAliasing;
} RaytracerShaderValues;

typedef struct RaytracerShaderLocations {
    int time;
    int resolution;
    int focalLength;
    int cameraCenter;
    int antiAliasing;
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

void Settings(RenderSettings *settings);
void DrawInfo(Camera camera, RenderSettings settings, int frame);

void CopyTexture(RenderTexture source, RenderTexture target, float resolution[2]);
void ClearTexture(RenderTexture tex);

#endif
