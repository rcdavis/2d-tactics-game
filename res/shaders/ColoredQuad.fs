// Colored Tile Fragment Shader
#version 460 core

layout(location = 0) in vec4 v_Color;

layout(location = 0) out vec4 color;

void main() {
	if (v_Color.a < 0.1)
		discard;
	color = v_Color;
}
