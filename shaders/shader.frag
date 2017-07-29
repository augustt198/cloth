#version 330 core

out vec4 FragColor;

in vec3 pos;
in vec2 parity;

uniform vec3 cameraPos;

void main() {
    //vec3 color = vec3(0.7, 0.0, (sin(2.0*pos.z)+1)/2.0);
    vec3 color; //0.5+(sin(2.0*pos.z)+1)/4.0 * vec3(1.0);
    if (parity.x < 0.5) {
        color = vec3(1.0);
    } else if (parity.x > 0.5) {
        color = vec3(0.0);
    }
    //FragColor = vec4(vec3(parity.x), 1.0);
    FragColor = vec4(color, 1.0);
}
