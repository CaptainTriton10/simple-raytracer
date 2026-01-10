#version 330

#define SPHERE 0
#define NONE -1

#define MAX_OBJECTS 64
#define POS_INFINITY 100000000

out vec4 finalColour;
uniform vec2 resolution;

uniform float focalLength;
uniform vec3 cameraCenter;

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

struct Sphere {
    vec3 pos;
    float radius;
};

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

vec3 CalculateRayDirection(vec2 pixelIndex) {
    float viewportHeight = 2.0;
    float viewportWidth = viewportHeight * (resolution.x / resolution.y);

    vec3 viewportU = vec3(viewportWidth, 0.0, 0.0);
    vec3 viewportV = vec3(0.0, viewportHeight, 0.0);

    vec3 pixelDeltaU = viewportU / resolution.x;
    vec3 pixelDeltaV = viewportV / resolution.y;

    vec3 viewportUpperLeft = cameraCenter - vec3(0.0, 0.0, focalLength) - viewportU / 2 - viewportV / 2;
    vec3 pixel00Loc = viewportUpperLeft + 0.5 * (pixelDeltaU + pixelDeltaV);

    vec3 pixelCenter = pixel00Loc + pixelIndex.x * pixelDeltaU + pixelIndex.y * pixelDeltaV;
    vec3 rayDirection = pixelCenter - cameraCenter;

    return rayDirection;
}

void main() {
    vec2 pixelIndex = gl_FragCoord.xy - vec2(0.5);
    vec3 rayDirection = CalculateRayDirection(pixelIndex);

    Hittable objects[MAX_OBJECTS];
    objects[0] = Hittable(SPHERE, vec4(0.0, 0.0, 0.0, 0.5), true);
    objects[1] = Hittable(SPHERE, vec4(1.0, 0.0, -2.0, 0.5), true);
    objects[2] = Hittable(SPHERE, vec4(0.0, -10.5, 0.0, 10.0), true);

    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (!objects[i].isActive) {
            objects[i] = Hittable(NONE, vec4(0.0), false);
        }
    }

    Ray ray = Ray(cameraCenter, rayDirection);
    finalColour = vec4(RayColour(ray, objects), 1.0);
}
