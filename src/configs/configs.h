#ifndef CONFIGS_H
#define CONFIGS_H

#include "../objects/objects.h"
#include "../textures/textures.h"
#include "../../include/tomlc17.h"

#define LAMBERTIAN 0
#define METAL 1
#define DIELECTRIC 2
#define EMISSIVE 3

toml_datum_t GetConfigParam(toml_result_t table, char *section, char *item, toml_type_t type);

void GetConfigVec3(toml_result_t table, float *vec, char *section, char *item);
void GetConfigVec2(toml_result_t table, float *vec, char *section, char *item);

ShaderMaterial GetConfigMaterial(toml_result_t table, char *name, Atlas atlas);
size_t GetConfigObject(Hittable *objects, toml_result_t table, char *name, Atlas atlas);

#endif
