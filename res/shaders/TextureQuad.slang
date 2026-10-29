// Textured Quad Shader

struct VertexInput {
	[[vk::location(0)]] float3 position;
	[[vk::location(1)]] float4 color;
	[[vk::location(2)]] float2 texCoord;
	[[vk::location(3)]] int texIndex;
};

struct VertexOutput {
	float4 position : SV_Position;
	[[vk::location(0)]] float4 color;
	[[vk::location(1)]] float2 texCoord;
	[[vk::location(2)]] nointerpolation int texIndex;
};

struct FragmentInput {
	[[vk::location(0)]] float4 color;
	[[vk::location(1)]] float2 texCoord;
	[[vk::location(2)]] nointerpolation int texIndex;
};

uniform float4x4 u_ViewProjection;
[[vk::binding(0, 0)]] Sampler2D<float4> u_Textures[32];

// Must match the distanceRange the MTSDF atlas was generated with.
static const float MsdfPxRange = 6.0;

float median(float r, float g, float b) {
	return max(min(r, g), min(max(r, g), b));
}

[shader("vertex")]
VertexOutput vertexMain(VertexInput input) {
	VertexOutput output;
	output.color = input.color;
	output.texCoord = input.texCoord;
	output.texIndex = input.texIndex;
	output.position = mul(u_ViewProjection, float4(input.position, 1.0));
	return output;
}

[shader("fragment")]
float4 fragmentMain(FragmentInput input) : SV_Target {
	// Negative texture indices mark MTSDF text.
	if (input.texIndex < 0) {
		int atlasIndex = -input.texIndex;
		float3 msd = u_Textures[NonUniformResourceIndex(atlasIndex)].Sample(input.texCoord).rgb;
		int textureWidth;
		int textureHeight;
		u_Textures[NonUniformResourceIndex(atlasIndex)].GetDimensions(textureWidth, textureHeight);
		float2 unitRange = float2(MsdfPxRange) / float2(textureWidth, textureHeight);
		float2 screenTexSize = float2(1.0) / fwidth(input.texCoord);
		float screenPxRange = max(0.5 * dot(unitRange, screenTexSize), 1.0);
		float screenPxDistance = screenPxRange * (median(msd.r, msd.g, msd.b) - 0.5);
		float alpha = clamp(screenPxDistance + 0.5, 0.0, 1.0) * input.color.a;
		if (alpha < 0.01)
			discard;
		return float4(input.color.rgb, alpha);
	}

	float4 finalColor = u_Textures[NonUniformResourceIndex(input.texIndex)].Sample(input.texCoord) * input.color;
	if (finalColor.a < 0.1)
		discard;
	return finalColor;
}
