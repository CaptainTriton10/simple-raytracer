#include "aabb.h"

float IntervalClamp(Interval i, float x) {
    if (x < i.min) return i.min;
    if (x > i.max) return i.max;

    return x;
}

Interval IntervalExpand(Interval i, float delta) {
    float padding = delta / 2.0f;

    return (Interval) {i.min - padding, i.max + padding};
}

void PadToMinimums(AABB *aabb) {
    float delta = 0.0001;

    float xSize = aabb->x.max - aabb->x.min;
    float ySize = aabb->y.max - aabb->y.min;
    float zSize = aabb->z.max - aabb->z.min;

    if (xSize < delta) aabb->x = IntervalExpand(aabb->x, delta);
    if (ySize < delta) aabb->y = IntervalExpand(aabb->y, delta);
    if (zSize < delta) aabb->z = IntervalExpand(aabb->z, delta);
}

void InitInterval(Interval *i, Interval a, Interval b) {
    i->min = a.min <= b.min ? a.min : b.min;
    i->max = a.max >= b.max ? a.max : b.max;
}

void InitAABB(AABB *aabb, Vector3 a, Vector3 b) {
    aabb->x = (a.x <= b.x) ? (Interval) {a.x, b.x} : (Interval) {b.x, a.x};
    aabb->y = (a.y <= b.y) ? (Interval) {a.y, b.y} : (Interval) {b.y, a.y};
    aabb->z = (a.z <= b.z) ? (Interval) {a.z, b.z} : (Interval) {b.z, a.z};

    PadToMinimums(aabb);
}

void InitAABB2(AABB *aabb, AABB box0, AABB box1) {
    InitInterval(&aabb->x, box0.x, box1.x);
    InitInterval(&aabb->y, box0.y, box1.y);
    InitInterval(&aabb->z, box0.z, box1.z);
}
