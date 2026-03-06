#include "configs.h"
#include "../utils/utils.h"
#include <stdlib.h>
#include <string.h>

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

static ShaderMaterial GetConfigMaterial(toml_result_t table, char *name, Atlas atlas) {
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

        bool isTextured = toml_seek(table.toptab, textureKey).type == TOML_STRING;
        if (isTextured) {
            toml_datum_t textureT = GetConfigParam(table, name, "texture", TOML_STRING);
            const char *textureS = _strdup(textureT.u.s);

            int index = GetTextureIndex(atlas, textureS);

            texture = index == -1 ? -1 : index;
        } else {
            texture = -1;   // No texture
            printf("No texture for %s\n", name);
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

size_t GetConfigObject(Hittable *objects, toml_result_t table, char *name, Atlas atlas) {
    toml_datum_t objType = GetConfigParam(table, name, "type", TOML_STRING);

    if (strcmp(objType.u.s, "sphere") == 0) {
        objects[0].type = SPHERE;

        float position3[3];
        GetConfigVec3(table, position3, name, "position");
        Vector3 position = {position3[0], position3[1], position3[2]};

        toml_datum_t radiusT = GetConfigParam(table, name, "radius", TOML_FP64);

        toml_datum_t materialT = GetConfigParam(table, name, "material", TOML_STRING);
        ShaderMaterial mat = GetConfigMaterial(table, (char*)materialT.u.s, atlas);

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
        ShaderMaterial mat = GetConfigMaterial(table, (char*)materialT.u.s, atlas);

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
        ShaderMaterial mat = GetConfigMaterial(table, (char*)materialT.u.s, atlas);

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

RenderSettings ParseRendererConfig(const char *filename) {
    const char headerName[] = "settings";
    toml_result_t result = toml_parse_file_ex(filename);

    if (!result.ok) {
        char errMsg[128];
        sprintf(errMsg, "%s config parse error.", filename);

        error(errMsg);
    }

    RenderSettings settings;

    toml_datum_t widthT = GetConfigParam(result, (char*)headerName, "width", TOML_INT64);
    if (widthT.u.int64 <= 0) {
        error("Width cannot be negative or 0.");
    }
    settings.width = widthT.u.int64;

    toml_datum_t heightT = GetConfigParam(result, (char*)headerName, "height", TOML_INT64);
    if (heightT.u.int64 <= 0) {
        error("Height cannot be negative or 0.");
    }
    settings.height = heightT.u.int64;

    toml_datum_t fullscreenT = GetConfigParam(result, (char*)headerName, "fullscreen", TOML_BOOLEAN);
    settings.fullscreen = fullscreenT.u.boolean;

    float fovLimits[2];
    GetConfigVec2(result, fovLimits, (char*)headerName, "fov_limits");
    if (fovLimits[0] > fovLimits[1]) {
        char errMsg[256];
        sprintf(errMsg, "The first argument of fov_limits cannot be greater than the second [%f > %f].", fovLimits[0], fovLimits[1]);

        error(errMsg);
    }


    toml_datum_t fovT = GetConfigParam(result, (char*)headerName, "fov", TOML_FP64);
    settings.fov = fovT.u.fp64;

    float camPos[3];
    GetConfigVec3(result, camPos, (char*)headerName, "camera_position");

    toml_datum_t aaT = GetConfigParam(result, (char*)headerName, "anti_aliasing", TOML_BOOLEAN);
    settings.aaEnabled = aaT.u.boolean;
    toml_datum_t maxDepthT = GetConfigParam(result, (char*)headerName, "max_depth", TOML_INT64);
    settings.maxDepth = maxDepthT.u.boolean;

    toml_datum_t texturesPathT = GetConfigParam(result, (char*)headerName, "textures_path", TOML_STRING);

    char key[128];
    sprintf(key, "%s.atlas_chunk_size", headerName);

    toml_datum_t atlasChunkSizeT = toml_seek(result.toptab, key);
    if (atlasChunkSizeT.type != TOML_INT64) {
        settings.atlasChunkSize = 512;
    } else {
        settings.atlasChunkSize = atlasChunkSizeT.u.int64;
    }

    settings.texturesPath = _strdup(texturesPathT.u.s);

    memcpy(settings.fovLimits, fovLimits, sizeof(fovLimits));
    memcpy(settings.cameraPosition, camPos, sizeof(camPos));

    return settings;
}

Scene ParseSceneConfig(const char *filename, Atlas atlas) {
    toml_result_t result = toml_parse_file_ex(filename);

    if (!result.ok) {
        error("Config parse error.");
    }

    // Get objects and materials
    toml_datum_t objectsT = GetConfigParam(result, "data", "objects", TOML_ARRAY);

    const size_t objCount = objectsT.u.arr.size;
    char *objNames[objCount];

    Hittable *objects = NULL;
    size_t objectSizes[objCount];

    size_t primativeCount = 0;

    // Get the names of all the objects
    for (int i = 0; i < objCount; i++) {
        if (objectsT.u.arr.elem[i].type == TOML_STRING){
            objNames[i] = _strdup(objectsT.u.arr.elem[i].u.s);

            Hittable objBuf[6];
            size_t objSize = GetConfigObject(objBuf, result, objNames[i], atlas);

            Hittable *newPtr = realloc(objects, sizeof(Hittable) * (primativeCount + objSize));
            if (!newPtr) {
                free(objects);
                error("Memory allocation failed");
            }

            for (int j = 0; j < objSize; j++) {
                newPtr[primativeCount + j] = objBuf[j];
            }

            primativeCount += objSize;

            objectSizes[i] = objSize;

            objects = newPtr;
        } else {
            error("Object name is not a string.");
        }
    }

    int count = 0;
    for (int i = 0; i < objCount; i++) {
        if (objectSizes[i] == 1) {
            objects[count].name = malloc(MAX_BUFFER_SIZE);
            strcpy(objects[count].name, objNames[i]);

            objects[count].index = count;
        } else {
            for (int j = 0; j < objectSizes[i]; j++) {
                char name[MAX_BUFFER_SIZE];
                sprintf(name, "%s [%d]", objNames[i], j + 1);

                objects[count + j].name = malloc(MAX_BUFFER_SIZE);
                strcpy(objects[count + j].name, name);

                objects[count + j].index = count + j;
            }
        }

        count += objectSizes[i];
    }

    float skyColour[3] = {1.0, 0.0, 1.0};
    if (toml_seek(result.toptab, "world.sky_colour").type == TOML_ARRAY) {
        GetConfigVec3(result, skyColour, "world", "sky_colour");
    }

    Scene scene = {
        .objCount = primativeCount,
        .objects = objects,
    };

    memcpy(scene.sky, skyColour, sizeof(skyColour));

    toml_free(result);

    for (int i = 0; i < objCount; i++) {
        free(objNames[i]);
    }

    return scene;
}