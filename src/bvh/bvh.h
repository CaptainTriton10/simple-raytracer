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

void ComputeWorldBBoxes(Scene *scene);
int InitBVHNode(Scene *scene, size_t start, size_t end, BVHNode *nodes);

Texture2D CreateBVHData(BVHNode *nodes, size_t nodeCount);
void CreateBVH(Scene *scene, BVHNode *nodes);

#endif
