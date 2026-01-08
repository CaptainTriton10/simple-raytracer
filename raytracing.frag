#version 330

out vec4 finalColour;
uniform vec2 resolution;

uniform float focalLength;
uniform vec3 cameraCenter;
// uniform vec2 viewport;

struct Ray {
    vec3 origin;
    vec3 direction;
};

float HitSphere(vec3 center, float radius, Ray ray) {
    vec3 oc = center - ray.origin;

    float a = dot(ray.direction, ray.direction);
    float b = -2.0 * dot(ray.direction, oc);
    float c = dot(oc, oc) - radius * radius;

    float discriminant = b * b - 4 * a * c;

    if (discriminant < 0) {
        return -1.0;
    } else {
        return (-b - sqrt(discriminant)) / (2.0 * a);
    }
}

vec3 At(Ray ray, float t) {
    return ray.origin + ray.direction * t;
}

vec4 RayColour(Ray ray) {
    float t = HitSphere(vec3(0.0, 0.0, -1.0), 0.5, ray);

    if (t > 0) {
        vec3 N = normalize(At(ray, t) - vec3(0.0, 0.0, -1.0));
        return 0.5 * vec4(N.x + 1.0, N.y + 1.0, N.z + 1.0, 1.0);
    }

    vec3 unitDirection = normalize(ray.direction);
    float a = 0.5 * (unitDirection.y + 1.0f);

    vec3 colour = mix(
            vec3(1.0, 1.0, 1.0),
            vec3(0.5, 0.7, 1.0),
            a
        );

    return vec4(colour, 1.0);
}

void main() {
    vec2 pixelIndex = gl_FragCoord.xy - vec2(0.5);

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

    Ray ray = Ray(cameraCenter, rayDirection);

    finalColour = RayColour(ray);
}
