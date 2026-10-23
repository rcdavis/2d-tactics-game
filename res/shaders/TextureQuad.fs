// Textured Quad Fragment Shader
#version 460 core

layout(location = 0) in vec4 v_Color;
layout(location = 1) in vec2 v_TexCoord;
layout(location = 2) in flat int v_TexIndex;

layout(binding = 0) uniform sampler2D u_Textures[32];

layout(location = 0) out vec4 color;

// Must match the distanceRange the MTSDF atlas was generated with
const float MsdfPxRange = 6.0;

float median(float r, float g, float b) {
	return max(min(r, g), min(max(r, g), b));
}

void main() {
	// Negative texture indices mark MTSDF text
	if (v_TexIndex < 0) {
		int atlasIndex = -v_TexIndex;
		vec3 msd = texture(u_Textures[atlasIndex], v_TexCoord).rgb;
		vec2 unitRange = vec2(MsdfPxRange) / vec2(textureSize(u_Textures[atlasIndex], 0));
		vec2 screenTexSize = vec2(1.0) / fwidth(v_TexCoord);
		float screenPxRange = max(0.5 * dot(unitRange, screenTexSize), 1.0);
		float screenPxDistance = screenPxRange * (median(msd.r, msd.g, msd.b) - 0.5);
		float alpha = clamp(screenPxDistance + 0.5, 0.0, 1.0) * v_Color.a;
		if (alpha < 0.01)
			discard;
		color = vec4(v_Color.rgb, alpha);
		return;
	}

    vec4 finalColor = texture(u_Textures[v_TexIndex], v_TexCoord) * v_Color;
	if (finalColor.a < 0.1)
		discard;
	color = finalColor;
}
