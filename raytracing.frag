#version 330

#define SPHERE 0
#define NONE -1

#define MAX_OBJECTS 64
#define POS_INFINITY 100000000

out vec4 finalColour;
uniform vec2 resolution;

uniform float focalLength;
uniform vec3 cameraCenter;

uniform int aaEnabled;

struct HitRecord {
    vec3 pos;
    vec3 normal;
    float t;
    bool frontFace;
};

struct Interval {
    float min;
    float max;
};

struct Hittable {
    int type; // Object type (0 for sphere)
    vec4 data; // e.g Sphere: xyz = pos, w = radius
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
};

float RandomNormalised(vec2 seed) {
    return fract(sin(dot(seed, vec2(12.9898, 78.233))) * 43758.5453);
}

float Random(vec2 seed, float min, float max) {
    return min + (max - min) * RandomNormalised(seed);
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

void SetFaceNormal(inout HitRecord rec, Ray ray, vec3 outwardNormal) {
    rec.frontFace = dot(ray.direction, outwardNormal) < 0;
    rec.normal = rec.frontFace ? outwardNormal : -outwardNormal;
}

float LengthSquared(vec3 v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
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
    vec3 outwardNormal = (temp.pos - sphere.pos) / sphere.radius;

    SetFaceNormal(temp, ray, outwardNormal);

    rec = temp;

    return true;
}

bool HitHittable(Hittable object, Ray ray, Interval rayT, out HitRecord rec) {
    if (object.type == SPHERE) {
        Sphere sphere = Sphere(object.data.xyz, object.data.w);

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
    HitRecord rec;
    if (HitWorld(ray, Interval(0, POS_INFINITY), rec, objects)) {
        return 0.5 * (rec.normal + vec3(1.0, 1.0, 1.0));
    }

    vec3 unitDirection = normalize(ray.direction);
    float a = 0.5 * (unitDirection.y + 1.0f);

    vec3 colour = mix(
            vec3(1.0, 1.0, 1.0),
            vec3(0.5, 0.7, 1.0),
            a
        );

    return colour;
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
    vec2 seed = gl_FragCoord.xy + vec2(index * 17, index * 31);
    return vec3(
        RandomNormalised(seed) - 0.5,
        RandomNormalised(seed.yx) - 0.5,
        0.0
    );
}

Ray GetRay(Camera camera, vec2 pixelIndex, int index) {
    vec3 offset = SampleSquare(index);
    vec3 pixelSample = camera.pixel00Loc
            + ((pixelIndex.x + offset.x) * camera.pixelDeltaU)
            + ((pixelIndex.y + offset.y) * camera.pixelDeltaV);

    vec3 rayOrigin = camera.position;
    vec3 rayDirection = pixelSample - rayOrigin;

    return Ray(rayOrigin, rayDirection);
}

void main() {
    vec2 pixelIndex = gl_FragCoord.xy - vec2(0.5);

    Camera camera;
    camera.focalLength = focalLength;
    camera.position = cameraCenter;
    camera.samplesPerPixel = 20;

    InitialiseCamera(camera);

    Hittable objects[MAX_OBJECTS];

    // Create some objects
    objects[0] = Hittable(SPHERE, vec4(0.0, 0.0, 0.0, 0.5), true);
    objects[1] = Hittable(SPHERE, vec4(1.0, 0.0, -2.0, 0.5), true);
    objects[2] = Hittable(SPHERE, vec4(0.0, -10.5, 0.0, 10.0), true);

    // Fill the rest as empty
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (!objects[i].isActive) {
            objects[i] = Hittable(NONE, vec4(0.0), false);
        }
    }

    if (aaEnabled == 1) {
        vec3 pixelColour = vec3(0.0, 0.0, 0.0);
        for (int i = 0; i < camera.samplesPerPixel; i++) {
            Ray ray = GetRay(camera, pixelIndex, i);
            pixelColour += RayColour(ray, objects);
        }

        pixelColour /= camera.samplesPerPixel;
        finalColour = vec4(pixelColour, 1.0);
    } else {
        vec3 rayDirection = CalculateRayDirection(camera, pixelIndex);
        Ray ray = Ray(cameraCenter, rayDirection);
        finalColour = vec4(RayColour(ray, objects), 1.0);
    }
}
