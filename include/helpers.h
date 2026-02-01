#ifndef HELPERS_H
#define HELPERS_H

#include "raylib.h"
#include "../include/tomlc17.h"
#include <stddef.h>

typedef struct ShaderMaterial {
    int type;
    float albedo[3];
    float roughness;
    float ior;
} ShaderMaterial;

typedef struct Sphere {
    float pos[3];
    float radius;
    ShaderMaterial material;
} Sphere;

typedef struct Scene {
    Sphere *objects;
    size_t objCount;
} Scene;

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
    float *forward;
    float *right;
    float *up;
    int antiAliasing;
    int dataSize;
} RaytracerShaderValues;

typedef struct RaytracerShaderLocations {
    int time;
    int resolution;
    int focalLength;
    int cameraCenter;
    int forward;
    int right;
    int up;
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

typedef struct BasisVectors {
    float forward[3];
    float right[3];
    float up[3];
} BasisVectors;

void error(const char *msg);
void SceneFree(Scene *scene);

toml_datum_t GetConfigParam(toml_result_t table, char *section, char *item, toml_type_t type);
void GetConfigVec3(toml_result_t table, float *vec, char *section, char *item);
Sphere GetObjectParams(toml_result_t table, char *name);

RaytracerShaderLocations GetRaytracerLocations(Shader shader);
void SetRaytracerValues(Shader shader, RaytracerShaderLocations locs, RaytracerShaderValues values);

DenoiserShaderLocations GetDenoiserLocations(Shader shader);
void SetDenoiserValues(Shader shader, DenoiserShaderLocations locs, DenoiserShaderValues values);

float Clampf(float value, float min, float max);
void NormaliseVec3(float v[3]);
void CrossVec3(float v[3], float a[3], float b[3]);

bool Movement(Camera *camera, BasisVectors vectors);
bool Zoom(Camera *camera);
BasisVectors Look(float *yaw, float *pitch);

bool Settings(RenderSettings *settings);
void DrawInfo(Camera camera, RenderSettings settings, int frame, Vector2 cameraRotation);

void CopyTexture(RenderTexture source, RenderTexture target, float resolution[2]);
void ClearTexture(RenderTexture tex);

#endif
