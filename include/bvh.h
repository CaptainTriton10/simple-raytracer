#ifndef BVH_H
#define BVH_H

#include "raylib.h"
#include "raymath.h"
#include "helpers.h"
#include <stddef.h>
#include <stdlib.h>

float IntervalClamp(Interval i, float x) {
    if (x < i.min) return i.min;
    if (x > i.max) return i.max;

    return x;
}

Interval IntervalExpand(Interval i, float delta) {
    float padding = delta / 2.0f;

    return (Interval) {i.min - padding, i.max + padding};
}

void InitInterval(Interval *i, Interval a, Interval b) {
    i->min = a.min <= b.min ? a.min : b.min;
    i->max = a.max >= b.max ? a.max : b.max;
}

void InitAABB(AABB *aabb, Vector3 a, Vector3 b) {
    aabb->x = (a.x <= b.x) ? (Interval) {a.x, b.x} : (Interval) {b.x, a.x};
    aabb->y = (a.y <= b.y) ? (Interval) {a.y, b.y} : (Interval) {b.y, a.y};
    aabb->z = (a.z <= b.z) ? (Interval) {a.z, b.z} : (Interval) {b.z, a.z};
}

void InitAABB2(AABB *aabb, AABB box0, AABB box1) {
    InitInterval(&aabb->x, box0.x, box1.x);
    InitInterval(&aabb->y, box0.y, box1.y);
    InitInterval(&aabb->z, box0.z, box1.z);
}

void CalculateSphereBBox(Sphere *s) {
    Vector3 rvec = {s->radius, s->radius, s->radius};
    Vector3 staticCenter = {s->pos[0], s->pos[1], s->pos[2]};

    InitAABB(&s->bbox, Vector3Subtract(staticCenter, rvec),  Vector3Add(staticCenter, rvec));
}

AABB ComputeSpanBBox(Scene *scene, int start, int end) {
    AABB box = scene->objects[start].bbox;

    for (int i = start + 1; i < end; i++) {
        InitAABB2(&box, box, scene->objects[i].bbox);
    }

    return box;
}

void ComputeWorldBBoxes(Scene *scene) {
    for (int i = 0; i < scene->objCount; i++) {
        CalculateSphereBBox(&scene->objects[i]);

        InitAABB2(&scene->bbox, scene->bbox, scene->objects[i].bbox);
    }
}

Interval AxisInterval(AABB aabb, int n) {
    if (n == 1) return aabb.y;
    if (n == 2) return aabb.z;
    return aabb.x;
}

int BoxCompare(void *context, const void *a, const void *b) {
    int axis = *(int*) context;
    Sphere *sa = (Sphere*) a;
    Sphere *sb = (Sphere*) b;

    Interval ia = AxisInterval(sa->bbox, axis);
    Interval ib = AxisInterval(sb->bbox, axis);

    if (ia.min < ib.min) return -1;
    if (ia.min > ib.min) return 1;
    return 0;
}

int InitBVHNode(Scene *scene, size_t start, size_t end) {
    int axis = GetRandomValue(0, 2);

    int nodeIndex = scene->nodeCount++;
    BVHNode *node = &scene->nodes[nodeIndex];

    size_t objectSpan = end - start;

    AABB spanBox = ComputeSpanBBox(scene, start, end);
    node->bbox = spanBox;

    if (objectSpan == 1) {
        node->left = (HittableRef) {HITTABLE_SPHERE, start};
        node->right = (HittableRef) {HITTABLE_SPHERE, start};
    } else if (objectSpan == 2) {
        node->left = (HittableRef) {HITTABLE_SPHERE, start};
        node->right = (HittableRef) {HITTABLE_SPHERE, start + 1};
    } else {
        qsort_s(&scene->objects[start], objectSpan, sizeof(Sphere), BoxCompare, &axis);

        int mid = objectSpan / 2.0f;

        int leftNode = InitBVHNode(scene, start, mid);
        int rightNode = InitBVHNode(scene, mid, end);

        node->left = (HittableRef) {HITTABLE_BVH_NODE, leftNode};
        node->right = (HittableRef) {HITTABLE_BVH_NODE, rightNode};
    }

    return nodeIndex;
}

#endif
