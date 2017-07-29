#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aParity;

uniform mat4 cameraDir;
uniform mat4 perspective;

out vec3 pos;
out vec2 parity;

void main() {
    pos = aPos;
    parity = aParity;

    gl_Position = perspective * cameraDir * vec4(0.25*aPos, 1.0);
    //gl_Position = vec4(aPos/4.0, 1.0);
}
