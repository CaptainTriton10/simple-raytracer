#ifndef OBJECTS_H
#define OBJECTS_H

#include "../aabb/aabb.h"
#include <raylib.h>
#include <stddef.h>

#define DATA_WIDTH 5

#define NONE -1
#define BVH_NODE 0
#define SPHERE 1
#define QUAD 2
#define CUBE 3

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
} Scene;

Hittable TranslateSphereData(Sphere s);
Sphere HittableToSphere(Hittable h);

Hittable TranslateQuadData(Quad q);
Quad HittableToQuad(Hittable h);

void CubeToQuads(Quad *quads, Cube cube);

#endif
