#ifndef BVH_H
#define BVH_H

#include "../aabb/aabb.h"
#include "../objects/objects.h"
#include <raylib.h>
#include <stddef.h>

typedef struct HittableRef {
    int type;
    int index;
} HittableRef;

typedef struct BVHNode {
    AABB bbox;
    HittableRef left;
    HittableRef right;
} BVHNode;

void CalculateSphereBBox(Sphere *s);
void CalculateQuadBBox(Quad *quad);

AABB ComputeSpanBBox(Scene *scene, int start, int end);
void ComputeWorldBBoxes(Scene *scene);

Interval AxisInterval(AABB aabb, int n);
int LongestAxis(AABB aabb);

int BoxCompare(void *context, const void *a, const void *b);

int InitBVHNode(Scene *scene, size_t start, size_t end, BVHNode *nodes);

#endif
