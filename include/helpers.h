#ifndef HELPERS_H
#define HELPERS_H

#include "raylib.h"
#include "../include/tomlc17.h"
#include <stddef.h>

#define DATA_WIDTH 5
#define MAX_LOOP_DEPTH 128

#define NONE -1
#define BVH_NODE 0
#define SPHERE 1
#define QUAD 2
#define CUBE 3

#define LAMBERTIAN 0
#define METAL 1
#define DIELECTRIC 2
#define EMISSIVE 3

typedef struct Interval {
    float min;
    float max;
} Interval;

typedef struct HittableRef {
    int type;
    int index;
} HittableRef;

typedef struct AABB {
    Interval x, y, z;
} AABB;

typedef struct BVHNode {
    AABB bbox;
    HittableRef left;
    HittableRef right;
} BVHNode;

typedef struct ShaderMaterial {
    int type;
    float albedo[3];
    int texture;
    float emission[3];
    float roughness;
    float ior;
} ShaderMaterial;

typedef struct Sphere {
    Vector3 pos;
    float radius;
    ShaderMaterial material;
    AABB bbox;
} Sphere;

typedef struct Quad {
    Vector3 Q;
    Vector3 u, v;
    ShaderMaterial material;
    AABB bbox;
} Quad;

typedef struct Cube {
    Vector3 a;
    Vector3 b;
    ShaderMaterial material;
} Cube;

typedef struct Hittable {
    int type;
    Vector4 data[DATA_WIDTH];
    AABB bbox;
} Hittable;

typedef struct Scene {
    Hittable *objects;
    size_t objCount;
    float sky[3];
    AABB bbox;
    int nodeCount;
    BVHNode *nodes;
} Scene;

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

typedef struct BasisVectors {
    float forward[3];
    float right[3];
    float up[3];
} BasisVectors;

void error(const char *msg);
char *ReplaceSubstr(char *s, char *orig, char *s2);
void SceneFree(Scene *scene);

Hittable TranslateSphereData(Sphere s);
Sphere HittableToSphere(Hittable h);

Hittable TranslateQuadData(Quad q);
Quad HittableToQuad(Hittable h);

toml_datum_t GetConfigParam(toml_result_t table, char *section, char *item, toml_type_t type);
void GetConfigVec3(toml_result_t table, float *vec, char *section, char *item);
void GetConfigVec2(toml_result_t table, float *vec, char *section, char *item);

ShaderMaterial GetConfigMaterial(toml_result_t table, char *name, Atlas atlas);
size_t GetConfigObject(Hittable *objects, toml_result_t table, char *name, Atlas atlas);

RaytracerShaderLocations GetRaytracerLocations(Shader shader);
void SetRaytracerValues(Shader shader, RaytracerShaderLocations locs, RaytracerShaderValues values);

DenoiserShaderLocations GetDenoiserLocations(Shader shader);
void SetDenoiserValues(Shader shader, DenoiserShaderLocations locs, DenoiserShaderValues values);

void NormaliseVec3(float v[3]);
void CrossVec3(float v[3], float a[3], float b[3]);

bool Movement(Camera *camera, BasisVectors vectors);
bool Zoom(Camera *camera, RenderSettings settings);
BasisVectors Look(float *yaw, float *pitch);

bool Settings(RenderSettings *settings);
void DrawInfo(Camera camera, RenderSettings settings, int frame, Vector2 cameraRotation);

void CopyTexture(RenderTexture source, RenderTexture target, float resolution[2]);
void ClearTexture(RenderTexture tex);

#endif
