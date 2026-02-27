#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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

void SceneFree(Scene *scene) {
    if (!scene) return;

    free(scene->objects);
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
