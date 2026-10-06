// Textured Quad Fragment Shader
#version 460 core

layout(location = 0) in vec4 v_Color;
layout(location = 1) in vec2 v_TexCoord;
layout(location = 2) in flat int v_TexIndex;

layout(binding = 0) uniform sampler2D u_Textures[32];

layout(location = 0) out vec4 color;

void main() {
    vec4 finalColor = texture(u_Textures[v_TexIndex], v_TexCoord) * v_Color;
	if (finalColor.a < 0.1)
		discard;
	color = finalColor;
}
