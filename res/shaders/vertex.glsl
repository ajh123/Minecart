#version 400 core

in uint vertex;

out vec3 passColor;
out vec3 passLight;
out float passInWater;

uniform mat4 transformationMatrix;
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform vec3 cameraPosition;
uniform float viewDistance;
uniform float inWater;

const vec3[6] normalVectors = vec3[6](
    vec3( 1,  0,  0),
    vec3(-1,  0,  0),
    vec3( 0,  1,  0),
    vec3( 0, -1,  0),
    vec3( 0,  0,  1),
    vec3( 0,  0, -1)
);

const float[6] lights = float[6](
    0.85,
    0.85,
    1.00,
    0.50,
    0.70,
    0.70
);

out float distance;
out float faceLight;
out float passViewDistance;

void main() {

    float y = (vertex >> 55u) & 0x1FFu;
    float x = (vertex >> 46u) & 0x1FFu;
    float z = (vertex >> 37u) & 0x1FFu;

    uint xUv = (vertex & 0x100u) >> 8u;
    uint yUv = (vertex & 0x80u) >> 7u;
    uint zUv = vertex & 0x7Fu;
    // FIXME: 02/02/2023 Temporary light value
    int light = 5;

    vec3 position = vec3(x, y, z);

    vec4 worldPosition = transformationMatrix * vec4(position, 1.0);
    vec4 positionRelativeToCam = viewMatrix * worldPosition;

    gl_Position = projectionMatrix * positionRelativeToCam;
    passColor = vec3(xUv, yUv, zUv);
    passLight = normalVectors[light];

    distance = length(cameraPosition.xz - worldPosition.xz);

    faceLight = lights[5];
    passViewDistance = viewDistance;
    passInWater = inWater;
}