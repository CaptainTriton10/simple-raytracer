#include "objects.h"
#include <math.h>
#include <string.h>
#include <raymath.h>
#include <stdlib.h>

Hittable TranslateSphereData(Sphere s) {
    Hittable h;
    h.type = SPHERE;

    h.data[0].x = SPHERE;

    h.data[1].x = s.pos.x;
    h.data[1].y = s.pos.y;
    h.data[1].z = s.pos.z;
    h.data[1].w = s.radius;

    h.data[2].x = s.material.type;
    h.data[2].y = s.material.albedo[0];
    h.data[2].z = s.material.albedo[1];
    h.data[2].w = s.material.albedo[2];

    h.data[3].x = s.material.roughness;
    h.data[3].y = s.material.ior;
    h.data[3].z = s.material.texture;

    h.data[4].x = s.material.emission[0];
    h.data[4].y = s.material.emission[1];
    h.data[4].z = s.material.emission[2];

    return h;
}

Sphere HittableToSphere(Hittable h) {
    float albedo[3] = {h.data[2].y, h.data[2].z, h.data[2].w};
    float emission[3] = {h.data[4].x, h.data[4].y, h.data[4].z};

    ShaderMaterial mat = {
        .type = h.data[2].x,
        .texture = h.data[3].z,
        .roughness = h.data[3].z,
        .ior = h.data[3].y
    };

    memcpy(mat.albedo, albedo, sizeof(albedo));
    memcpy(mat.emission, emission, sizeof(emission));

    Vector3 position = {
        h.data[1].x,
        h.data[1].y,
        h.data[1].z,
    };

    Sphere s = {
        .radius = h.data[1].w,
        .material = mat,
        .pos = position
    };

    return s;
}

Hittable TranslateQuadData(Quad q) {
    Hittable h = {0};
    h.type = QUAD;

    h.data[0].x = QUAD;
    h.data[0].y = q.Q.x;
    h.data[0].z = q.Q.y;
    h.data[0].w = q.Q.z;

    h.data[1].x = q.u.x;
    h.data[1].y = q.u.y;
    h.data[1].z = q.u.z;
    h.data[1].w = q.material.ior;

    h.data[2].x = q.v.x;
    h.data[2].y = q.v.y;
    h.data[2].z = q.v.z;
    h.data[2].w = q.material.roughness;

    h.data[3].x = q.material.type;
    h.data[3].y = q.material.albedo[0];
    h.data[3].z = q.material.albedo[1];
    h.data[3].w = q.material.albedo[2];

    h.data[4].x = q.material.emission[0];
    h.data[4].y = q.material.emission[1];
    h.data[4].z = q.material.emission[2];
    h.data[4].w = q.material.texture;

    return h;
}

Quad HittableToQuad(Hittable h) {
    float albedo[3] = {h.data[3].y, h.data[3].z, h.data[3].w};
    float emission[3] = {h.data[4].x, h.data[4].y, h.data[4].z};

    ShaderMaterial mat = {
        .type = h.data[3].x,
        .texture = h.data[4].w,
        .ior = h.data[1].w,
        .roughness = h.data[2].w
    };

    memcpy(mat.albedo, albedo, sizeof(albedo));
    memcpy(mat.emission, emission, sizeof(emission));

    Vector3 q = {h.data[0].y, h.data[0].z, h.data[0].w};
    Vector3 u = {h.data[1].x, h.data[1].y, h.data[1].z};
    Vector3 v = {h.data[2].x, h.data[2].y, h.data[2].z};

    Quad quad = {
        .Q = q,
        .u = u,
        .v = v,
        .material = mat
    };

    return quad;
}

void CubeToQuads(Quad *quads, Cube cube) {
    Vector3 min = {
        fmin(cube.a.x, cube.b.x),
        fmin(cube.a.y, cube.b.y),
        fmin(cube.a.z, cube.b.z),
    };

    Vector3 max = {
        fmax(cube.a.x, cube.b.x),
        fmax(cube.a.y, cube.b.y),
        fmax(cube.a.z, cube.b.z),
    };

    Vector3 dx = {max.x - min.x, 0.0, 0.0};
    Vector3 dy = {0.0, max.y - min.y, 0.0};
    Vector3 dz = {0.0, 0.0, max.z - min.z};

    quads[0] = (Quad){
        .Q = {min.x, min.y, max.z},
        .u = dx,
        .v = dy,
        .material = cube.material
    };

    quads[1] = (Quad){
        .Q = {max.x, min.y, max.z},
        .u = Vector3Negate(dz),
        .v = dy,
        .material = cube.material
    };

    quads[2] = (Quad){
        .Q = {max.x, min.y, min.z},
        .u = Vector3Negate(dx),
        .v = dy,
        .material = cube.material
    };

    quads[3] = (Quad){
        .Q = {min.x, min.y, min.z},
        .u = dz,
        .v = dy,
        .material = cube.material
    };

    quads[4] = (Quad){
        .Q = {min.x, max.y, max.z},
        .u = dx,
        .v = Vector3Negate(dz),
        .material = cube.material
    };

    quads[5] = (Quad){
        .Q = {min.x, min.y, min.z},
        .u = dx,
        .v = dz,
        .material = cube.material
    };
}

Texture2D CreateSceneData(Hittable objects[], size_t len) {
    size_t dataSize = len * DATA_WIDTH * 4;
    float *data = malloc(dataSize * sizeof(float));

    for (int i = 0; i < len; i++) {
        int base = i * DATA_WIDTH * 4;

        for (int j = 0; j < DATA_WIDTH; j++) {
            data[base + j * 4 + 0] = objects[i].data[j].x;
            data[base + j * 4 + 1] = objects[i].data[j].y;
            data[base + j * 4 + 2] = objects[i].data[j].z;
            data[base + j * 4 + 3] = objects[i].data[j].w;
        }
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

void SceneFree(Scene *scene) {
    if (!scene) return;

    for (int i = 0; i < scene->objCount; i++) {
        free(scene->names[i]);
    }

    free(scene->objects);
}
