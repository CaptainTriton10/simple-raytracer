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

static float IntervalClamp(Interval i, float x);
static Interval IntervalExpand(Interval i, float delta);

static void PadToMinimums(AABB *aabb);

static void InitInterval(Interval *i, Interval a, Interval b);
void InitAABB(AABB *aabb, Vector3 a, Vector3 b);
void InitAABB2(AABB *aabb, AABB box0, AABB box1);

#endif
