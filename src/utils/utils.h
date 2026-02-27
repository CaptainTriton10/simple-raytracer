#ifndef UTILS_H
#define UTILS_H

#include "../objects/objects.h"
#include <raylib.h>

void error(const char *msg);

char *ReplaceSubstr(char *s, char *orig, char *s2);

void SceneFree(Scene *scene);   // TODO: remove

void NormaliseVec3(float *v);
void CrossVec3(float *v, float *a, float *b);
void Vec3ToArray(float *arr, Vector3 vec);

void CopyTexture(RenderTexture source, RenderTexture target, float resolution[2]);
void ClearTexture(RenderTexture tex);

#endif
