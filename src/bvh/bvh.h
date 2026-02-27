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

static void CalculateSphereBBox(Sphere *s);
static void CalculateQuadBBox(Quad *quad);

static AABB ComputeSpanBBox(Scene *scene, int start, int end);
void ComputeWorldBBoxes(Scene *scene);

static Interval AxisInterval(AABB aabb, int n);
static int LongestAxis(AABB aabb);

static int BoxCompare(void *context, const void *a, const void *b);

int InitBVHNode(Scene *scene, size_t start, size_t end, BVHNode *nodes);

#endif
