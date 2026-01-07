#version 330

out vec4 finalColour;
uniform vec2 resolution;

uniform float focalLength;
// uniform vec3 cameraCenter;
// uniform vec2 viewport;

struct Ray {
    vec3 origin;
    vec3 direction;
};

vec4 RayColour(Ray ray) {
    vec3 unitDirection = normalize(ray.direction);
    float a = 0.5 * (unitDirection.y + 1.0f);

    vec3 colour = mix(
            vec3(1.0, 1.0, 1.0),
            vec3(0.5, 0.7, 1.0),
            a
        );

    return vec4(colour, 1.0);
}

vec3 At(Ray ray, float t) {
    return ray.origin + ray.direction * t;
}

void main() {
    vec3 cameraCenter = vec3(0.0, 0.0, 0.0);

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
