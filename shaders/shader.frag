#version 450

layout(set = 0, binding = 1) uniform sampler2D textures[32];

layout(push_constant) uniform Push {
    layout(offset = 64) int textureIndex;
} push;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(textures[push.textureIndex], fragTexCoord) * vec4(fragColor, 1.0);
}
