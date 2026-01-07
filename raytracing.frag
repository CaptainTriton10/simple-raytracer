#version 330

out vec4 finalColour;
uniform vec2 resolution;

void main() {
    vec2 uv = gl_FragCoord.xy / resolution;

    finalColour = vec4(uv.x, uv.y, 0.0, 1.0);
}
