#ifndef AABB_H
#define AABB_H

#include <raylib.h>

typedef struct Interval {
    float min;
    float max;
} Interval;

typedef struct AABB {
    Interval x, y, z;
} AABB;

void InitAABB(AABB *aabb, Vector3 a, Vector3 b);
void InitAABB2(AABB *aabb, AABB box0, AABB box1);

#endif
