#ifndef BVH_H
#define BVH_H

#include "raylib.h"
#include "raymath.h"
#include "helpers.h"
#include <stddef.h>
#include <stdio.h>
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

void CalculateSphereBBox(Sphere *s) {
    Vector3 rvec = {s->radius, s->radius, s->radius};
    Vector3 staticCenter = {s->pos.x, s->pos.y, s->pos.z};

    InitAABB(&s->bbox, Vector3Subtract(staticCenter, rvec),  Vector3Add(staticCenter, rvec));
}

void CalculateQuadBBox(Quad *quad) {
    Vector3 q = {quad->Q.x, quad->Q.y, quad->Q.z};

    Vector3 u = {quad->u.x, quad->u.y, quad->u.z};
    Vector3 v = {quad->v.x, quad->v.y, quad->v.z};

    AABB diagonal1;
    InitAABB(&diagonal1, q, Vector3Add(q, Vector3Add(u, v)));

    AABB diagonal2;
    InitAABB(&diagonal2, Vector3Add(q, u), Vector3Add(q, v));

    InitAABB2(&quad->bbox, diagonal1, diagonal2);
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
        if (scene->objects[i].type == SPHERE) {
            Sphere s = HittableToSphere(scene->objects[i]);

            CalculateSphereBBox(&s);
            scene->objects[i].bbox = s.bbox;
        } else if (scene->objects[i].type == QUAD) {
            Quad q = HittableToQuad(scene->objects[i]);

            CalculateQuadBBox(&q);
            scene->objects[i].bbox = q.bbox;
        }

        // InitAABB2(&scene->bbox, scene->bbox, scene->objects[i].bbox);
    }
}

Interval AxisInterval(AABB aabb, int n) {
    if (n == 1) return aabb.y;
    if (n == 2) return aabb.z;
    return aabb.x;
}

int LongestAxis(AABB aabb) {
    float xSize = aabb.x.max - aabb.x.min;
    float ySize = aabb.y.max - aabb.y.min;
    float zSize = aabb.z.max - aabb.z.min;

    if (xSize > ySize) return xSize > zSize ? 0 : 2;
    else return ySize > zSize ? 1 : 2;
}

int BoxCompare(void *context, const void *a, const void *b) {
    int axis = *(int*) context;
    Hittable *sa = (Hittable*) a;
    Hittable *sb = (Hittable*) b;

    Interval ia = AxisInterval(sa->bbox, axis);
    Interval ib = AxisInterval(sb->bbox, axis);

    if (ia.min < ib.min) return -1;
    if (ia.min > ib.min) return 1;
    return 0;
}

int InitBVHNode(Scene *scene, size_t start, size_t end) {
    int nodeIndex = scene->nodeCount++;
    BVHNode *node = &scene->nodes[nodeIndex];

    size_t objectSpan = end - start;

    AABB spanBox = ComputeSpanBBox(scene, start, end);
    int axis = LongestAxis(spanBox);

    node->bbox = spanBox;

    if (objectSpan == 1) {
        node->left = (HittableRef) {scene->objects[start].type, start};
        node->right = (HittableRef) { NONE, -1};
    } else if (objectSpan == 2) {
        node->left = (HittableRef) {scene->objects[start].type, start};
        node->right = (HittableRef) {scene->objects[start].type, start + 1};
    } else {
        qsort_s(&scene->objects[start], objectSpan, sizeof(Hittable), BoxCompare, &axis);

        int mid = start + objectSpan / 2;

        int leftNode = InitBVHNode(scene, start, mid);
        int rightNode = InitBVHNode(scene, mid, end);

        node->left = (HittableRef) {BVH_NODE, leftNode};
        node->right = (HittableRef) {BVH_NODE, rightNode};
    }

    return nodeIndex;
}

#endif
