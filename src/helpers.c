#include "../include/helpers.h"
#include "../include/tomlc17.h"
#include "raylib.h"
#include "raymath.h"
#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define CAMERA_MOVE_SPEED 1.5
#define CAMERA_ZOOM_SPEED 7
#define MOUSE_SENSETIVITY 0.5

void error(const char *msg) {
    fprintf(stderr, "ERROR: %s\n", msg);
    exit(1);
}

char *ReplaceSubstr(char *s, char *orig, char *s2) {
    char *pos = strstr(s, orig);
    if (!pos) {
        fprintf(stderr, "WARNING: pattern [%s] not found in string.\n", orig);
        return 0;
    }

    size_t sLen = strlen(s);
    size_t origLen = strlen(orig);
    size_t s2Len = strlen(s2);

    size_t newSize = sLen - origLen + s2Len;
    char *buf = malloc(newSize + 1);

    // Copy part before substring
    size_t prefixLen = pos - s;
    memcpy(buf, s, prefixLen);

    // Copy substring
    memcpy(buf + prefixLen, s2, s2Len);

    // Copy remainder
    memcpy(buf + prefixLen + s2Len, pos + origLen, sLen - origLen - prefixLen);

    return buf;
}

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

void GetConfigVec2(toml_result_t table, float *vec, char *section, char *item) {
    char path[64];
    sprintf(path, "%s.%s", section, item);

    toml_datum_t param = toml_seek(table.toptab, path);
    if (param.type != TOML_ARRAY) {
        char errMsg[128];
        sprintf(errMsg, "Missing or invalid %s property", path);

        error(errMsg);
    } else if(param.u.arr.size != 2) {
        char errMsg[128];
        sprintf(errMsg, "Wrong number of arguments (%d) for vec2 [%s]", param.u.arr.size, path);

        error(errMsg);
    }

    float result[2];

    for (int i = 0; i < 2; i++) {
        result[i] = (float) param.u.arr.elem[i].u.fp64;
    }

    memcpy(vec, result, sizeof(result));    // Move result to input float array
}

ShaderMaterial GetConfigMaterial(toml_result_t table, char *name) {
    toml_datum_t typeT = GetConfigParam(table, name, "type", TOML_STRING);
    char *typeS = _strdup(typeT.u.s);
    int type = 0;

    float albedo[3] = {0.0f, 0.0f, 0.0f};
    float emission[3] = {0.0f, 0.0f, 0.0f};

    float roughness = 0.0f;
    float ior = 0.0f;
    int texture = 0;

    if (strcmp(TextToUpper(typeS), "DIFFUSE") == 0) {
        type = LAMBERTIAN;

        GetConfigVec3(table, albedo, name, "albedo");

        char textureKey[64];
        sprintf(textureKey, "%s.texture", name);

        bool isTextured = toml_seek(table.toptab, textureKey).type == TOML_INT64;
        if (isTextured) {
            toml_datum_t textureT = GetConfigParam(table, name, "texture", TOML_INT64);
            texture = textureT.u.int64;
        } else {
            texture = -1;   // No texture
            printf("No texture for %s", name);
        }

    } else if (strcmp(TextToUpper(typeS), "METAL") == 0) {
        type = METAL;

        GetConfigVec3(table, albedo, name, "albedo");
        toml_datum_t roughnessT = GetConfigParam(table, name, "roughness", TOML_FP64);

        roughness = roughnessT.u.fp64;

    } else if (strcmp(TextToUpper(typeS), "GLASS") == 0) {
        type = DIELECTRIC;

        toml_datum_t iorT = GetConfigParam(table, name, "ior", TOML_FP64);

        ior = iorT.u.fp64;

    } else if (strcmp(TextToUpper(typeS), "EMISSIVE") == 0) {
        type = EMISSIVE;

        GetConfigVec3(table, emission, name, "emission");

    } else {
        char errMsg[128];
        sprintf(errMsg, "Unknown %s.type property [%s]", name, typeS);

        error(errMsg);
    }

    ShaderMaterial material = {
        .type = type,
        .texture = texture,
        .roughness = roughness,
        .ior = ior
    };

    memcpy(material.albedo, albedo, sizeof(albedo));
    memcpy(material.emission, emission, sizeof(emission));

    free(typeS);

    return material;
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

size_t GetConfigObject(Hittable *objects, toml_result_t table, char *name) {
    toml_datum_t objType = GetConfigParam(table, name, "type", TOML_STRING);

    if (strcmp(objType.u.s, "sphere") == 0) {
        objects[0].type = SPHERE;

        float position3[3];
        GetConfigVec3(table, position3, name, "position");
        Vector3 position = {position3[0], position3[1], position3[2]};

        toml_datum_t radiusT = GetConfigParam(table, name, "radius", TOML_FP64);

        toml_datum_t materialT = GetConfigParam(table, name, "material", TOML_STRING);
        ShaderMaterial mat = GetConfigMaterial(table, (char*)materialT.u.s);

        Sphere sphere = {
            .pos = position,
            .radius = radiusT.u.fp64,
            .material = mat
        };

        *objects = TranslateSphereData(sphere);
        return 1;
    } else if (strcmp(objType.u.s, "quad") == 0) {
        objects[0].type = QUAD;

        float q3[3];
        GetConfigVec3(table, q3, name, "Q");
        Vector3 q = {q3[0], q3[1], q3[2]};

        float u3[3];
        GetConfigVec3(table, u3, name, "u");
        Vector3 u = {u3[0], u3[1], u3[2]};

        float v3[3];
        GetConfigVec3(table, v3, name, "v");
        Vector3 v = {v3[0], v3[1], v3[2]};

        toml_datum_t materialT = GetConfigParam(table, name, "material", TOML_STRING);
        ShaderMaterial mat = GetConfigMaterial(table, (char*)materialT.u.s);

        Quad quad = {
            .material = mat,
            .Q = q,
            .u = u,
            .v = v
        };

        *objects = TranslateQuadData(quad);
        return 1;
    } else if (strcmp(objType.u.s, "cube") == 0) {
        toml_datum_t materialT = GetConfigParam(table, name, "material", TOML_STRING);
        ShaderMaterial mat = GetConfigMaterial(table, (char*)materialT.u.s);

        float a[3];
        GetConfigVec3(table, a, name, "a");

        float b[3];
        GetConfigVec3(table, b, name, "b");

        Cube cube = {
            .a = {a[0], a[1], a[2]},
            .b = {b[0], b[1], b[2]},
            .material = mat
        };

        Quad quads[6];
        CubeToQuads(quads, cube);

        for (int i = 0; i < 6; i++) {
            objects[i] = TranslateQuadData(quads[i]);
        }

        return 6;
    }

    char errMsg[128];
    sprintf(errMsg, "Object type unknown [%s]", objType.u.s);

    error(errMsg);
    return 0;
}

void SceneFree(Scene *scene) {
    if (!scene) return;

    free(scene->objects);
}

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

void NormaliseVec3(float *v) {
    float n[3];
    memcpy(n, v, sizeof(float) * 3);

    float magnitude = sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);

    n[0] /= magnitude;
    n[1] /= magnitude;
    n[2] /= magnitude;

    memcpy(v, n, sizeof(float) * 3);
}

void CrossVec3(float *v, float *a, float *b) {
    float n[3] = {
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0]
    };

    memcpy(v, n, sizeof(float) * 3);
}

void Vec3ToArray(float *arr, Vector3 vec) {
    arr[0] = vec.x;
    arr[1] = vec.y;
    arr[2] = vec.z;
}

bool Movement(Camera *camera, BasisVectors vectors) {
    float move = CAMERA_MOVE_SPEED * GetFrameTime();
    bool changed = false;

    if (IsKeyDown(KEY_W)) {
        camera->position.x += vectors.forward[0] * move;
        camera->position.y += vectors.forward[1] * move;
        camera->position.z += vectors.forward[2] * move;
        changed = true;
    }

    if (IsKeyDown(KEY_S)) {
        camera->position.x -= vectors.forward[0] * move;
        camera->position.y -= vectors.forward[1] * move;
        camera->position.z -= vectors.forward[2] * move;
        changed = true;
    }

    if (IsKeyDown(KEY_A)) {
        camera->position.x -= vectors.right[0] * move;
        camera->position.y -= vectors.right[1] * move;
        camera->position.z -= vectors.right[2] * move;
        changed = true;
    }


    if (IsKeyDown(KEY_D)) {
        camera->position.x += vectors.right[0] * move;
        camera->position.y += vectors.right[1] * move;
        camera->position.z += vectors.right[2] * move;
        changed = true;
    }

    if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_C)) {
        camera->position.x -= vectors.up[0] * move;
        camera->position.y -= vectors.up[1] * move;
        camera->position.z -= vectors.up[2] * move;
        changed = true;
    }

    if (IsKeyDown(KEY_SPACE)) {
        camera->position.x += vectors.up[0] * move;
        camera->position.y += vectors.up[1] * move;
        camera->position.z += vectors.up[2] * move;
        changed = true;
    }

    return changed;
}

bool Zoom(Camera *camera, RenderSettings settings) {
    float zoomFactor = CAMERA_ZOOM_SPEED * GetFrameTime();
    float scroll = 1 + zoomFactor * GetMouseWheelMove();

    camera->fovy = Clamp(camera->fovy / scroll, settings.fovLimits[0], settings.fovLimits[1]);

    // If the camera was zoomed this frame
    if (scroll != 1.0) {
        return true;
    }

    return false;
}

BasisVectors Look(float *yaw, float *pitch) {
    BasisVectors vectors;
    float worldUp[3] = {0.0, 1.0, 0.0};

    Vector2 mouseDelta = {
            GetMouseDelta().x * MOUSE_SENSETIVITY * GetFrameTime(),
            GetMouseDelta().y * MOUSE_SENSETIVITY * GetFrameTime()};

    *yaw -= mouseDelta.x;
    *pitch -= mouseDelta.y;
    *pitch = Clamp(*pitch, -85.0f * DEG2RAD, 85.0f * DEG2RAD);

    float forward[3] = {
        cosf(*pitch) * cosf(*yaw),
        sinf(*pitch),
        cosf(*pitch) * sinf(*yaw)
    };

    NormaliseVec3(forward);

    float right[3];
    CrossVec3(right, worldUp, forward);
    NormaliseVec3(right);

    float up[3];
    CrossVec3(up, forward, right);

    memcpy(vectors.forward, forward, sizeof(forward));
    memcpy(vectors.right, right, sizeof(right));
    memcpy(vectors.up, up, sizeof(up));

    return vectors;
}

bool Settings(RenderSettings *settings) {
    if (IsKeyPressed(KEY_ONE)) {
        settings->aaEnabled = settings->aaEnabled == 1 ? 0 : 1;
        return true;
    }

    return false;
}

void DrawInfo(Camera camera, RenderSettings settings, int frame, Vector2 cameraRotation) {
    char frameInfo[16];
    sprintf(frameInfo, "Frame: %d", frame);

    char cameraPosInfo[128];
    sprintf(cameraPosInfo, "Camera Position: [%.2f, %.2f, %.2f]", camera.position.x, camera.position.y, camera.position.z);

    char cameraRotInfo[128];
    sprintf(cameraRotInfo, "Camera Rotation: [yaw = %.2f, pitch = %.2f]", cameraRotation.x * RAD2DEG, cameraRotation.y * RAD2DEG);

    char cameraFovyInfo[64];
    sprintf(cameraFovyInfo, "Camera Focal Length: %.2f", camera.fovy);

    char aaInfo[64];
    sprintf(aaInfo, "Anti-Aliasing: %d", settings.aaEnabled);

    DrawFPS(5, 5);

    DrawText(cameraPosInfo, 5, 50, 20, RED);
    DrawText(cameraRotInfo, 5, 75, 20, RED);
    DrawText(cameraFovyInfo, 5, 100, 20, RED);

    DrawText(aaInfo, 5, 150, 20, YELLOW);

    DrawText(frameInfo, 5, 200, 20, PURPLE);
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
