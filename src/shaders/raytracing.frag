#version 330

#define BVH 0
#define SPHERE 1
#define QUAD 2
#define NONE -1

#define LAMBERTIAN 0
#define METAL 1
#define DIELECTRIC 2
#define EMISSIVE 3

#define MAX_OBJECTS 10
#define MAX_BVH_STACK 10

#define POS_INFINITY 100000000
#define PI 3.1415926

out vec4 finalColour;
uniform vec2 resolution;
uniform float time;

uniform sampler2D data;
uniform int dataSize;

uniform sampler2D bvhData;

uniform sampler2D textureAtlas;
uniform int chunkSize;

uniform float fov;
uniform vec3 cameraCenter;

uniform vec3 skyColour;

uniform vec3 forward;
uniform vec3 right;
uniform vec3 up;

uniform int aaEnabled;
uniform int maxDepth;

struct Material {
    int type;
    vec3 albedo;
    int texture;
    vec3 emission;
    float roughness;
    float ior;
};

struct HitRecord {
    vec3 pos;
    vec3 normal;
    Material material;
    float t;
    bool frontFace;
    vec2 uv;
};

struct Interval {
    float min;
    float max;
};

struct BVHNode {
    vec3 minB;
    vec3 maxB;
    int leftType;
    int leftIndex;
    int rightType;
    int rightIndex;
};

/*

Hittable Data Types:
Sphere:
    data0   xyz = pos, w = radius
    data1   x = scatter type, yzw = albedo
    data2   x = roughness, y = ior
    data3   xyz = emission

*/
struct Hittable {
    int type;
    vec4 data0, data1, data2, data3, data4;
    bool isActive;
};

struct Ray {
    vec3 origin;
    vec3 direction;
};

struct Camera {
    int imageHeight;
    float focalLength;
    float fov;

    int samplesPerPixel;
    float pixelSamplesScale;

    float defocusAngle;
    float focus;

    vec3 position;

    vec3 forward;
    vec3 right;
    vec3 up;

    vec3 pixel00Loc;
    vec3 pixelDeltaU;
    vec3 pixelDeltaV;
};

struct Sphere {
    vec3 pos;
    float radius;
    Material material;
};

struct Quad {
    vec3 Q;
    vec3 u;
    vec3 v;
    vec3 normal;
    float D;
    vec3 w;
    Material material;
};

float LengthSquared(vec3 v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

float Random(vec2 p)
{
    ivec2 i = ivec2(p);
    uint h = uint(i.x) * 1664525u + uint(i.y) * 1013904223u;
    h ^= h >> 16;
    h *= 2246822519u;
    return float(h) / float(0xffffffffu);
}

float Random(vec2 seed, float min, float max) {
    return min + (max - min) * Random(seed);
}

vec3 RandomVec3(vec2 seed) {
    return vec3(
        Random(seed * vec2(13.0, 17.0)),
        Random(seed * vec2(31.0, 37.0)),
        Random(seed * vec2(51.0, 97.0)));
}

vec3 RandomVec3(vec2 seed, float min, float max) {
    return vec3(
        Random(seed * vec2(13.0, 17.0), min, max),
        Random(seed * vec2(31.0, 37.0), min, max),
        Random(seed * vec2(51.0, 97.0), min, max));
}

vec3 RandomUnitVec3(vec2 seed) {
    for (int i = 0; i < 16; i++) {
        vec3 p = RandomVec3(seed + i * 13.0, -1.0, 1.0);
        float lensq = LengthSquared(p);

        if (lensq <= 1 && lensq > 1e-45) {
            return p / sqrt(lensq);
        }
    }

    return vec3(1.0, 0.0, 0.0);
}

vec3 RandomOnHemisphere(vec3 normal, vec2 seed) {
    vec3 onUnitSphere = RandomUnitVec3(seed);
    if (dot(onUnitSphere, normal) > 0.0) {
        return onUnitSphere;
    } else {
        return -onUnitSphere;
    }
}

vec3 RandomUnitDisk(vec2 seed) {
    vec3 p = vec3(
            Random(seed, -1.0, 1.0),
            Random(vec2(seed.x * 17, seed.y * 31), -1.0, 1.0),
            0.0);

    if (LengthSquared(p) < 1.0) {
        return p;
    } else {
        return normalize(p);
    }
}

vec3 Reflect(vec3 v, vec3 n) {
    return v - 2 * dot(v, n) * n;
}

vec3 Refract(vec3 uv, vec3 n, float etaIOverEtaT) {
    float cosTheta = min(dot(-uv, n), 1.0);
    vec3 rOutPerp = etaIOverEtaT * (uv + cosTheta * n);
    vec3 rOutParallel = -sqrt(abs(1.0 - LengthSquared(rOutPerp))) * n;

    return rOutPerp + rOutParallel;
}

float Reflectance(float cosine, float ior) {
    float r0 = (1.0 - ior) / (1.0 + ior);
    r0 = r0 * r0;

    return r0 + (1.0 - r0) * pow((1.0 - cosine), 5.0);
}

bool NearZero(vec3 a) {
    float s = 1e-8;
    return (abs(a.x) < s) && (abs(a.y) < s) && (abs(a.z) < s);
}

vec3 At(Ray ray, float t) {
    return ray.origin + ray.direction * t;
}

float IntervalSize(Interval interval) {
    return interval.max - interval.min;
}

bool IntervalContains(Interval interval, float x) {
    return interval.min <= x && interval.max >= x;
}

bool IntervalSurrounds(Interval interval, float x) {
    return interval.min < x && interval.max > x;
}

float IntervalClamp(Interval interval, float x) {
    if (x < interval.min) return interval.min;
    if (x > interval.max) return interval.max;
    return x;
}

vec3 CheckerTexture(vec2 uv, vec3 p, float scale, vec3 even, vec3 odd) {
    float invScale = 1.0 / scale;

    int xInteger = int(floor(invScale * p.x));
    int yInteger = int(floor(invScale * p.y));
    int zInteger = int(floor(invScale * p.z));

    bool isEven = (xInteger + yInteger + zInteger) % 2 == 0;

    return isEven ? even : odd;
}

vec3 SampleAtlas(int index, vec2 uv, vec3 p) {
    float u = IntervalClamp(Interval(0.0, 1.0), uv.x);
    float v = 1.0 - IntervalClamp(Interval(0.0, 1.0), uv.y);

    vec2 size = textureSize(textureAtlas, 0);
    int divisions = int(size.x) / chunkSize;

    int tileX = index % divisions;
    int tileY = index / divisions;

    int baseX = tileX * chunkSize;
    int baseY = tileY * chunkSize;

    int i = int(u * (chunkSize - 1)) + baseX;
    int j = int(v * (chunkSize - 1)) + baseY;

    vec3 pixel = texelFetch(textureAtlas, ivec2(i, j), 0).rgb;

    return pixel;
}

bool LambertianScatter(Material mat, Ray ray, HitRecord rec, inout vec3 attenuation, inout Ray scattered) {
    vec3 scatterDirection = rec.normal + RandomUnitVec3(gl_FragCoord.xy * (gl_FragCoord.yx * time));

    if (NearZero(scatterDirection)) {
        scatterDirection = rec.normal;
    }

    scattered = Ray(rec.pos, scatterDirection);

    attenuation = SampleAtlas(rec.material.texture, rec.uv, rec.pos);

    return true;
}

bool MetalScatter(Material mat, Ray ray, HitRecord rec, inout vec3 attenuation, inout Ray scattered) {
    vec3 reflected = Reflect(ray.direction, rec.normal);
    reflected = normalize(reflected) + (mat.roughness * RandomUnitVec3(gl_FragCoord.xy * (gl_FragCoord.yx * time)));
    scattered = Ray(rec.pos, reflected);
    attenuation = mat.albedo;

    return dot(scattered.direction, rec.normal) > 0;
}

bool DielectricScatter(Material mat, Ray ray, HitRecord rec, inout vec3 attenuation, inout Ray scattered) {
    attenuation = vec3(1.0, 1.0, 1.0);
    float ri = rec.frontFace ? (1.0 / mat.ior) : mat.ior;

    vec3 unitDirection = normalize(ray.direction);
    float cosTheta = min(dot(-unitDirection, rec.normal), 1.0);
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    bool cannotRefract = ri * sinTheta > 1.0;
    vec3 direction = vec3(0.0);

    if (cannotRefract || Reflectance(cosTheta, mat.ior) > Random(gl_FragCoord.xy * (gl_FragCoord.yx * time))) {
        direction = Reflect(unitDirection, rec.normal);
    } else {
        direction = Refract(unitDirection, rec.normal, ri);
    }

    scattered = Ray(rec.pos, direction);
    return true;
}

void SetFaceNormal(inout HitRecord rec, Ray ray, vec3 outwardNormal) {
    rec.frontFace = dot(ray.direction, outwardNormal) < 0;
    rec.normal = rec.frontFace ? outwardNormal : -outwardNormal;
}

void GetSphereUV(vec3 p, out vec2 uv) {
    float theta = acos(-p.y);
    float phi = atan(-p.z, p.x) + PI;

    uv.x = phi / (2.0 * PI);
    uv.y = theta / PI;
}

Quad InitQuad(vec3 Q, vec3 u, vec3 v, Material mat) {
    Quad quad;
    quad.Q = Q;
    quad.u = u;
    quad.v = v;
    quad.material = mat;

    vec3 n = cross(u, v);
    quad.normal = normalize(n);
    quad.D = dot(quad.normal, Q);
    quad.w = n / dot(n, n);

    return quad;
}

bool HitSphere(Sphere sphere, Ray ray, Interval rayT, inout HitRecord rec) {
    vec3 oc = sphere.pos - ray.origin;

    float a = LengthSquared(ray.direction);
    float h = dot(ray.direction, oc);
    float c = LengthSquared(oc) - sphere.radius * sphere.radius;

    float discriminant = h * h - a * c;
    if (discriminant < 0) {
        return false;
    }

    float sqrtd = sqrt(discriminant);

    float root = (h - sqrtd) / a;
    if (!IntervalSurrounds(rayT, root)) {
        root = (h + sqrtd) / a;
        if (!IntervalSurrounds(rayT, root)) {
            return false;
        }
    }

    HitRecord temp;
    temp.t = root;
    temp.pos = At(ray, temp.t);
    temp.material = sphere.material;
    vec3 outwardNormal = (temp.pos - sphere.pos) / sphere.radius;

    GetSphereUV(outwardNormal, temp.uv);
    SetFaceNormal(temp, ray, outwardNormal);

    rec = temp;

    return true;
}

bool QuadIsInterior(Quad quad, float a, float b, inout HitRecord rec) {
    Interval unitInterval = Interval(0, 1);

    if (!IntervalContains(unitInterval, a) || !IntervalContains(unitInterval, b)) return false;

    rec.uv.x = a;
    rec.uv.y = b;

    return true;
}

bool HitQuad(Quad quad, Ray ray, Interval rayT, inout HitRecord rec) {
    float denom = dot(quad.normal, ray.direction);

    if (abs(denom) < 1e-8) return false;

    float t = (quad.D - dot(quad.normal, ray.origin)) / denom;
    if (!IntervalContains(rayT, t)) return false;

    vec3 intersection = At(ray, t);
    vec3 planarHitptVector = intersection - quad.Q;
    float alpha = dot(quad.w, cross(planarHitptVector, quad.v));
    float beta = dot(quad.w, cross(quad.u, planarHitptVector));

    if (!QuadIsInterior(quad, alpha, beta, rec)) return false;

    rec.t = t;
    rec.pos = intersection;
    rec.material = quad.material;
    SetFaceNormal(rec, ray, quad.normal);

    return true;
}

bool HitAABB(vec3 minB, vec3 maxB, Ray ray, float tMin, float tMax) {
    vec3 invD = 1.0 / ray.direction;
    vec3 t0 = (minB - ray.origin) * invD;
    vec3 t1 = (maxB - ray.origin) * invD;

    vec3 tsmaller = min(t0, t1);
    vec3 tbigger = max(t0, t1);

    tMin = max(tMin, max(tsmaller.x, max(tsmaller.y, tsmaller.z)));
    tMax = min(tMax, min(tbigger.x, min(tbigger.y, tbigger.z)));

    return tMax >= tMin;
}

bool HitHittable(Hittable object, Ray ray, Interval rayT, out HitRecord rec) {
    if (object.type == SPHERE) {
        Material mat = Material(
                int(object.data2.x), // Material type
                object.data2.yzw, // Albedo
                int(object.data3.z),
                object.data4.xyz, // Emission
                object.data3.x, // Roughness
                object.data3.y // IOR
            );
        Sphere sphere = Sphere(object.data1.xyz, object.data1.w, mat);

        return HitSphere(sphere, ray, rayT, rec);
    }
    if (object.type == QUAD) {
        Material mat = Material(
                int(object.data3.x),
                object.data3.yzw,
                int(object.data4.w),
                object.data4.xyz,
                object.data2.w,
                object.data1.w
            );

        vec3 Q = object.data0.yzw;
        vec3 u = object.data1.xyz;
        vec3 v = object.data2.xyz;

        Quad quad = InitQuad(Q, u, v, mat);

        return HitQuad(quad, ray, rayT, rec);
    }
    else if (object.type == NONE) {
        // Do nothing
        return false;
    }
}

bool HitWorld(Ray ray, Interval rayT, out HitRecord rec, Hittable objects[MAX_OBJECTS]) {
    HitRecord temp;
    bool hit = false;
    float closest = rayT.max;

    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (HitHittable(objects[i], ray, Interval(rayT.min, closest), temp) && objects[i].isActive) {
            hit = true;
            closest = temp.t;
            rec = temp;
        }
    }

    return hit;
}

Hittable GetHittable(int i) {
    Hittable object;

    object.data0 = texelFetch(data, ivec2(0, i), 0);
    object.data1 = texelFetch(data, ivec2(1, i), 0);
    object.data2 = texelFetch(data, ivec2(2, i), 0);
    object.data3 = texelFetch(data, ivec2(3, i), 0);
    object.data4 = texelFetch(data, ivec2(4, i), 0);

    object.isActive = true;
    object.type = int(object.data0.x);

    return object;
}

BVHNode GetBVHNode(int i) {
    vec4 t0 = texelFetch(bvhData, ivec2(0, i), 0);
    vec4 t1 = texelFetch(bvhData, ivec2(1, i), 0);
    vec4 t2 = texelFetch(bvhData, ivec2(2, i), 0);

    BVHNode node;
    node.minB = t0.xyz;
    node.maxB = t1.xyz;

    node.leftType = int(t2.x);
    node.leftIndex = int(t2.y);

    node.rightType = int(t2.z);
    node.rightIndex = int(t2.w);

    return node;
}

float NodeCenterDistance(BVHNode n, Ray ray) {
    vec3 center = (n.minB + n.maxB) / 2.0;
    return dot(center - ray.origin, ray.direction);
}

bool HitBVH(Ray ray, out HitRecord rec) {
    bool hitAnything = false;
    float closest = POS_INFINITY;

    int stack[MAX_BVH_STACK];
    int sp = 0;

    stack[sp++] = 0;

    while (sp > 0) {
        int nodeIndex = stack[--sp];
        BVHNode node = GetBVHNode(nodeIndex);

        if (!HitAABB(node.minB, node.maxB, ray, 0.001, closest)) continue;

        if (node.leftType == BVH && node.rightType == BVH) {
            BVHNode left = GetBVHNode(node.leftIndex);
            BVHNode right = GetBVHNode(node.rightIndex);

            bool leftFirst = NodeCenterDistance(left, ray) < NodeCenterDistance(right, ray);

            stack[sp++] = leftFirst ? node.rightIndex : node.leftIndex;
            stack[sp++] = leftFirst ? node.leftIndex : node.rightIndex;
        }

        if (node.leftType != BVH) {
            HitRecord temp;

            if (HitHittable(GetHittable(node.leftIndex), ray, Interval(0.001, closest), temp)) {
                hitAnything = true;
                closest = temp.t;
                rec = temp;
            }
        }

        if (node.rightType != BVH) {
            HitRecord temp;

            if (HitHittable(GetHittable(node.rightIndex), ray, Interval(0.001, closest), temp)) {
                hitAnything = true;
                closest = temp.t;
                rec = temp;
            }
        }
    }

    return hitAnything;
}

vec3 RayColour(Ray ray, Hittable objects[MAX_OBJECTS]) {
    vec3 attenuationAccum = vec3(1.0);
    Ray currentRay = ray;

    for (int i = 0; i < maxDepth; i++) {
        HitRecord rec;

        if (HitBVH(currentRay, rec)) {
            Ray scattered;
            vec3 attenuation;
            bool didScatter = false;

            vec3 emissiveColour = vec3(0.0);

            if (rec.material.type == LAMBERTIAN) {
                didScatter = LambertianScatter(
                        rec.material,
                        currentRay,
                        rec,
                        attenuation,
                        scattered
                    );
            } else if (rec.material.type == METAL) {
                didScatter = MetalScatter(
                        rec.material,
                        currentRay,
                        rec,
                        attenuation,
                        scattered
                    );
            } else if (rec.material.type == DIELECTRIC) {
                didScatter = DielectricScatter(
                        rec.material,
                        currentRay,
                        rec,
                        attenuation,
                        scattered
                    );
            } else if (rec.material.type == EMISSIVE) {
                return attenuationAccum * rec.material.emission;
            } else {
                didScatter = false;
            }

            if (!didScatter) {
                return vec3(0.0);
            }

            vec3 scatterColour = attenuation * attenuationAccum;
            attenuationAccum = scatterColour + emissiveColour;
            currentRay = scattered;
        } else {
            vec3 unitDirection = normalize(currentRay.direction);
            float a = 0.5 * (unitDirection.y + 1.0f);

            vec3 sky = skyColour;

            return attenuationAccum * sky;
        }
    }

    return vec3(0.0);
}

vec3 DefocusDiskSample(Camera camera, float lensRadius) {
    vec3 p = RandomUnitDisk(gl_FragCoord.xy * (gl_FragCoord.yx * time));
    return camera.position
        + p.x * lensRadius * camera.right
        + p.y * lensRadius * camera.up;
}

vec3 CalculateRayDirection(Camera camera) {
    vec2 uv = (gl_FragCoord.xy / resolution) * 2.0 - 1.0;
    uv.x *= resolution.x / resolution.y;

    float scale = tan(radians(camera.fov) / 2);

    vec3 rayDirection = normalize(
            camera.forward +
                uv.x * scale * camera.right +
                uv.y * scale * camera.up
        );

    return rayDirection;
}

vec3 SampleSquare(int index) {
    vec2 seed = gl_FragCoord.xy + vec2(index * 17.0, index * 31.0);
    return vec3(
        Random(seed) - 0.5,
        Random(seed.yx) - 0.5,
        0.0
    );
}

// Index = the current sample index
Ray GetRay(Camera camera, vec2 pixelIndex, int index) {
    vec3 offset = SampleSquare(index);
    vec3 pixelSample = camera.pixel00Loc
            + ((pixelIndex.x + offset.x) * camera.pixelDeltaU)
            + ((pixelIndex.y + offset.y) * camera.pixelDeltaV);

    vec3 rayOrigin = camera.position;
    vec3 rayDirection = pixelSample - rayOrigin;

    return Ray(rayOrigin, rayDirection);
}

vec3 LinearToGamma(vec3 colour) {
    vec3 result;
    if (colour.x > 0) {
        result.x = sqrt(colour.x);
    }

    if (colour.y > 0) {
        result.y = sqrt(colour.y);
    }

    if (colour.z > 0) {
        result.z = sqrt(colour.z);
    }

    return result;
}

float CalculateFocusDistance(Camera camera, Hittable objects[MAX_OBJECTS]) {
    Ray ray = Ray(camera.position, camera.forward);
    HitRecord rec;

    if (HitWorld(ray, Interval(0.0001, POS_INFINITY), rec, objects)) {
        return rec.t;
    } else {
        return 1.0;
    }
}

void main() {
    vec2 pixelIndex = gl_FragCoord.xy - vec2(0.5);

    Camera camera;
    camera.pixelSamplesScale = 1.0 / camera.samplesPerPixel;
    camera.defocusAngle = 0.4;

    camera.forward = forward;
    camera.right = right;
    camera.up = up;

    camera.fov = fov;
    camera.position = cameraCenter;
    camera.samplesPerPixel = 20;

    Hittable objects[MAX_OBJECTS];

    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (i <= dataSize) objects[i] = GetHittable(i);
        else {
            objects[i].type = NONE;
            objects[i].isActive = false;
        }
    }

    camera.focus = CalculateFocusDistance(camera, objects);

    if (aaEnabled == 1) {
        vec3 pixelColour = vec3(0.0, 0.0, 0.0);
        for (int i = 0; i < camera.samplesPerPixel; i++) {
            Ray ray = GetRay(camera, pixelIndex, i);
            pixelColour += RayColour(ray, objects);
        }

        pixelColour /= camera.samplesPerPixel;
        finalColour = vec4(LinearToGamma(pixelColour), 1.0);
    } else {
        vec3 rayDirection = CalculateRayDirection(camera);

        vec3 focusPoint = camera.position + rayDirection * camera.focus;
        float defocusRadius = camera.focus * tan(radians(camera.defocusAngle / 2.0));

        vec3 rayOrigin = (camera.defocusAngle <= 0.0) ? camera.position : DefocusDiskSample(camera, defocusRadius);

        vec3 finalDirection = normalize(focusPoint - rayOrigin);

        Ray ray = Ray(rayOrigin, finalDirection);
        finalColour = vec4(LinearToGamma(RayColour(ray, objects)), 1.0);
    }
}
