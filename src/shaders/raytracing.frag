#version 330

#define SPHERE 0
#define NONE -1

#define LAMBERTIAN 0
#define METAL 1

#define MAX_OBJECTS 3
#define POS_INFINITY 100000000

#define MAX_DEPTH 5

out vec4 finalColour;
uniform vec2 resolution;
uniform float time;

uniform float focalLength;
uniform vec3 cameraCenter;

uniform int aaEnabled;

struct Material {
    int type;
    vec3 albedo;
};

struct HitRecord {
    vec3 pos;
    vec3 normal;
    Material material;
    float t;
    bool frontFace;
};

struct Interval {
    float min;
    float max;
};

/*

Hittable Data Types:
Sphere:
data0   xyz = pos, w = radius
data1   x = scatter type, yzw = albedo

for now I am using:
data1   xyz = albedo, x = scatter type

I must remember to update this in the future

*/
struct Hittable {
    int type; // Object type (0 for sphere)
    vec4 data0; // e.g Sphere: xyz = pos, w = radius
    vec4 data1; // e.g Sphere: xyz = material albedo
    bool isActive;
};

struct Ray {
    vec3 origin;
    vec3 direction;
};

struct Camera {
    int imageHeight;
    float focalLength;

    int samplesPerPixel;
    float pixelSamplesScale;

    vec3 position;
    vec3 pixel00Loc;
    vec3 pixelDeltaU;
    vec3 pixelDeltaV;
};

struct Sphere {
    vec3 pos;
    float radius;
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

vec3 Reflect(vec3 v, vec3 n) {
    return v - 2 * dot(v, n) * n;
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

bool LambertianScatter(Material mat, Ray ray, HitRecord rec, inout vec3 attenuation, inout Ray scattered) {
    vec3 scatterDirection = rec.normal + RandomUnitVec3(gl_FragCoord.xy * (gl_FragCoord.yx * time));

    if (NearZero(scatterDirection)) {
        scatterDirection = rec.normal;
    }

    scattered = Ray(rec.pos, scatterDirection);
    attenuation = mat.albedo;

    return true;
}

bool MetalScatter(Material mat, Ray ray, HitRecord rec, inout vec3 attenuation, inout Ray scattered) {
    vec3 reflected = Reflect(ray.direction, rec.normal);
    scattered = Ray(rec.pos, reflected);
    attenuation = mat.albedo;
    return true;
}

void SetFaceNormal(inout HitRecord rec, Ray ray, vec3 outwardNormal) {
    rec.frontFace = dot(ray.direction, outwardNormal) < 0;
    rec.normal = rec.frontFace ? outwardNormal : -outwardNormal;
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

    SetFaceNormal(temp, ray, outwardNormal);

    rec = temp;

    return true;
}

bool HitHittable(Hittable object, Ray ray, Interval rayT, out HitRecord rec) {
    if (object.type == SPHERE) {
        Material mat = Material(int(object.data1.w), object.data1.xyz);
        Sphere sphere = Sphere(object.data0.xyz, object.data0.w, mat);

        return HitSphere(sphere, ray, rayT, rec);
    } else if (object.type == NONE) {
        // Do nothing
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

vec3 RayColour(Ray ray, Hittable objects[MAX_OBJECTS]) {
    vec3 attenuationAccum = vec3(1.0);
    Ray currentRay = ray;

    for (int i = 0; i < MAX_DEPTH; i++) {
        HitRecord rec;

        if (HitWorld(currentRay, Interval(0.0001, POS_INFINITY), rec, objects)) {
            Ray scattered;
            vec3 attenuation;
            bool didScatter = false;

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
            }

            if (!didScatter) {
                return vec3(0.0);
            }

            attenuationAccum *= attenuation;
            currentRay = scattered;
        } else {
            vec3 unitDirection = normalize(ray.direction);
            float a = 0.5 * (unitDirection.y + 1.0f);

            vec3 sky = mix(
                    vec3(1.0, 1.0, 1.0),
                    vec3(0.5, 0.7, 1.0),
                    a
                );

            return attenuationAccum * sky;
        }
    }

    return vec3(0.0);
}

void InitialiseCamera(inout Camera camera) {
    camera.pixelSamplesScale = 1.0 / camera.samplesPerPixel;

    float viewportHeight = 2.0;
    float viewportWidth = viewportHeight * (resolution.x / resolution.y);

    vec3 viewportU = vec3(viewportWidth, 0.0, 0.0);
    vec3 viewportV = vec3(0.0, viewportHeight, 0.0);

    camera.pixelDeltaU = viewportU / resolution.x;
    camera.pixelDeltaV = viewportV / resolution.y;

    vec3 viewportUpperLeft = camera.position - vec3(0.0, 0.0, camera.focalLength) - viewportU / 2 - viewportV / 2;
    camera.pixel00Loc = viewportUpperLeft + 0.5 * (camera.pixelDeltaU + camera.pixelDeltaV);
}

vec3 CalculateRayDirection(Camera camera, vec2 pixelIndex) {
    vec3 pixelCenter = camera.pixel00Loc + pixelIndex.x * camera.pixelDeltaU + pixelIndex.y * camera.pixelDeltaV;
    vec3 rayDirection = pixelCenter - camera.position;

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

void main() {
    vec2 pixelIndex = gl_FragCoord.xy - vec2(0.5);

    Camera camera;
    camera.focalLength = focalLength;
    camera.position = cameraCenter;
    camera.samplesPerPixel = 20;

    InitialiseCamera(camera);

    Hittable objects[MAX_OBJECTS];

    Material redMat = Material(1, vec3(0.7, 0.1, 0.1));
    Material blueMat = Material(1, vec3(0.1, 0.1, 0.7));
    Material greenMat = Material(0, vec3(0.1, 0.7, 0.1));
    Material whiteMat = Material(0, vec3(0.95, 0.95, 0.95));
    Material greyMat = Material(0, vec3(0.1, 0.1, 0.15));

    // Create some objects
    objects[0] = Hittable(SPHERE, vec4(0.0, 0.0, 0.0, 0.5), vec4(blueMat.albedo, blueMat.type), true);
    objects[1] = Hittable(SPHERE, vec4(1.0, 0.0, 1.25, 0.5), vec4(greenMat.albedo, greenMat.type), true);
    objects[2] = Hittable(SPHERE, vec4(0.0, -10.5, 0.0, 10.0), vec4(greyMat.albedo, greyMat.type), true);

    // Fill the rest as empty
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (!objects[i].isActive) {
            objects[i] = Hittable(NONE, vec4(0.0), vec4(0.0), false);
        }
    }

    if (aaEnabled == 1) {
        vec3 pixelColour = vec3(0.0, 0.0, 0.0);
        for (int i = 0; i < camera.samplesPerPixel; i++) {
            Ray ray = GetRay(camera, pixelIndex, i);
            pixelColour += RayColour(ray, objects);
        }

        pixelColour /= camera.samplesPerPixel;
        finalColour = vec4(LinearToGamma(pixelColour), 1.0);
    } else {
        vec3 rayDirection = CalculateRayDirection(camera, pixelIndex);
        Ray ray = Ray(cameraCenter, rayDirection);
        finalColour = vec4(LinearToGamma(RayColour(ray, objects)), 1.0);
    }
}
