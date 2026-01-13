#version 330

uniform vec2 resolution;

uniform sampler2D prevFrame;
uniform sampler2D accRender;

out vec4 finalColour;

void main() {
    vec2 uv = gl_FragCoord.xy / resolution;

    vec3 prevTex = texture(prevFrame, uv).rgb;
    vec3 accTex = texture(accRender, uv).rgb;

    finalColour = vec4(mix(accTex, prevTex, 0.05), 1.0);
}
