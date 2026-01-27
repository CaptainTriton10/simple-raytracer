#version 330

#define DENOISE_MAX_FRAMES -1

uniform vec2 resolution;

uniform sampler2D prevFrame;
uniform sampler2D accRender;

uniform int changed;
uniform int frame;

out vec4 finalColour;

void main() {
    vec2 uv = gl_FragCoord.xy / resolution;

    vec3 prevTex = texture(prevFrame, uv).rgb;
    vec3 accTex = texture(accRender, uv).rgb;

    float factor = 1.0 / float(frame + 1.0);

    if (frame >= DENOISE_MAX_FRAMES && DENOISE_MAX_FRAMES != -1) {
        factor = 0.0;
    }

    if (changed == 0) {
        finalColour = vec4(mix(accTex, prevTex, factor), 1.0);
    } else {
        finalColour = vec4(prevTex, 1.0);
    }
}
