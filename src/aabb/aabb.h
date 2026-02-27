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

float IntervalClamp(Interval i, float x);
Interval IntervalExpand(Interval i, float delta);

void PadToMinimums(AABB *aabb);

void InitInterval(Interval *i, Interval a, Interval b);
void InitAABB(AABB *aabb, Vector3 a, Vector3 b);
void InitAABB2(AABB *aabb, AABB box0, AABB box1);

#endif
