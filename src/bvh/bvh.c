#include "bvh.h"
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>

#define BVH_DATA_WIDTH 3

typedef struct BVHSortContext {
    Hittable *objects;
    int axis;
} BVHSortContext;

static void CalculateSphereBBox(Sphere *s) {
    Vector3 rvec = {s->radius, s->radius, s->radius};
    Vector3 staticCenter = {s->pos.x, s->pos.y, s->pos.z};

    InitAABB(&s->bbox, Vector3Subtract(staticCenter, rvec),  Vector3Add(staticCenter, rvec));
}

static void CalculateQuadBBox(Quad *quad) {
    Vector3 q = {quad->Q.x, quad->Q.y, quad->Q.z};

    Vector3 u = {quad->u.x, quad->u.y, quad->u.z};
    Vector3 v = {quad->v.x, quad->v.y, quad->v.z};

    AABB diagonal1;
    InitAABB(&diagonal1, q, Vector3Add(q, Vector3Add(u, v)));

    AABB diagonal2;
    InitAABB(&diagonal2, Vector3Add(q, u), Vector3Add(q, v));

    InitAABB2(&quad->bbox, diagonal1, diagonal2);
}

static AABB ComputeSpanBBox(Scene *scene, int start, int end) {
    AABB box = scene->objects[scene->indices[start]].bbox;

    for (int i = start + 1; i < end; i++) {
        InitAABB2(&box, box, scene->objects[scene->indices[i]].bbox);
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
    }
}

static Interval AxisInterval(AABB aabb, int n) {
    if (n == 1) return aabb.y;
    if (n == 2) return aabb.z;
    return aabb.x;
}

static int LongestAxis(AABB aabb) {
    float xSize = aabb.x.max - aabb.x.min;
    float ySize = aabb.y.max - aabb.y.min;
    float zSize = aabb.z.max - aabb.z.min;

    if (xSize > ySize) return xSize > zSize ? 0 : 2;
    else return ySize > zSize ? 1 : 2;
}

static int BoxCompare(void *_context, const void *a, const void *b) {
    BVHSortContext *context = (BVHSortContext*)_context;

    int axis = context->axis;

    int indexA = *(int*)a;
    int indexB = *(int*)b;

    Hittable *sa = &context->objects[indexA];
    Hittable *sb = &context->objects[indexB];

    Interval ia = AxisInterval(sa->bbox, axis);
    Interval ib = AxisInterval(sb->bbox, axis);

    if (ia.min < ib.min) return -1;
    if (ia.min > ib.min) return 1;
    return 0;
}

int InitBVHNode(Scene *scene, size_t start, size_t end, BVHNode *nodes) {
    int nodeIndex = scene->nodeCount++;
    BVHNode node;

    size_t objectSpan = end - start;

    AABB spanBox = ComputeSpanBBox(scene, start, end);
    int axis = LongestAxis(spanBox);

    node.bbox = spanBox;

    int leftIndex = scene->indices[start];
    int rightIndex = scene->indices[start + 1];

    if (objectSpan == 1) {
        node.left = (HittableRef) {scene->objects[scene->indices[start]].type, scene->indices[start]};
        node.right = (HittableRef) {NONE, -1};
    } else if (objectSpan == 2) {
        node.left = (HittableRef) {scene->objects[scene->indices[start]].type, scene->indices[start]};
        node.right = (HittableRef) {scene->objects[scene->indices[start + 1]].type, scene->indices[start + 1]};
    } else {
        BVHSortContext context = {
            .objects = scene->objects,
            .axis = axis
        };

        qsort_s(&scene->indices[start], objectSpan, sizeof(int), BoxCompare, &context);

        int mid = start + objectSpan / 2;

        int leftNode = InitBVHNode(scene, start, mid, nodes);
        int rightNode = InitBVHNode(scene, mid, end, nodes);

        node.left = (HittableRef) {BVH_NODE, leftNode};
        node.right = (HittableRef) {BVH_NODE, rightNode};
    }

    nodes[nodeIndex] = node;

    return nodeIndex;
}

Texture2D CreateBVHData(BVHNode *nodes, size_t nodeCount) {
    size_t dataSize = nodeCount * BVH_DATA_WIDTH * 4;
    float *data = malloc(dataSize * sizeof(float));

    for (int i = 0; i < nodeCount; i++) {
        int base = i * BVH_DATA_WIDTH * 4;

        data[base + 0] = nodes[i].bbox.x.min;
        data[base + 1] = nodes[i].bbox.y.min;
        data[base + 2] = nodes[i].bbox.z.min;
        data[base + 3] = 0.0f;

        data[base + 4] = nodes[i].bbox.x.max;
        data[base + 5] = nodes[i].bbox.y.max;
        data[base + 6] = nodes[i].bbox.z.max;
        data[base + 7] = 0.0f;

        data[base + 8] = nodes[i].left.type;
        data[base + 9] = nodes[i].left.index;
        data[base + 10] = nodes[i].right.type;
        data[base + 11] = nodes[i].right.index;
    }

    Image dataImage = {
        .data = data,
        .width = BVH_DATA_WIDTH,
        .height = nodeCount,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R32G32B32A32
    };

    Texture2D dataTexture = LoadTextureFromImage(dataImage);

    SetTextureFilter(dataTexture, TEXTURE_FILTER_POINT);
    SetTextureWrap(dataTexture, TEXTURE_WRAP_CLAMP);

    return dataTexture;
}

void CreateBVH(Scene *scene, BVHNode *nodes) {
    printf("Creating BVH...\n");
    double startTime = GetTime();

    ComputeWorldBBoxes(scene);

    scene->nodeCount = 0;

    int root = InitBVHNode(scene, 0, scene->objCount, nodes);

    printf("BVH created: %d nodes\n", scene->nodeCount);
    double totalTime = (GetTime() - startTime) * 1000;
    printf("BVH creation time: %fms\n", totalTime);
}