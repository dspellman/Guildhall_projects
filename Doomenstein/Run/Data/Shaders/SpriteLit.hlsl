struct vs_input_t
{
	float3 localPosition : POSITION;
	float3 normal : NORMAL;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};

struct v2p_t
{
	float4 position : SV_Position;
	float3 normal : NORMAL;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};

cbuffer VS_CONSTANT_BUFFER : register(b2)
{
	float4x4 ProjectionMatrix;
	float4x4 ViewMatrix;
};

cbuffer VS_MODEL_BUFFER : register(b3)
{
	float4x4 ModelMatrix;
	float4 ModelColor;
};

cbuffer VS_LIGHTING_BUFFER : register(b1)
{
	float3 sunDirection;
	float sunIntensity;
	float ambientIntensity;
	float3 padding;
};

Texture2D diffuseTexture : register(t0);
SamplerState diffuseSampler : register(s0);

v2p_t VertexMain(vs_input_t input)
{
	v2p_t v2p;
	float4 localPos = float4(input.localPosition, 1.0);
	float4 worldPos = mul(ModelMatrix, localPos);
	float4 viewPos = mul(ViewMatrix, worldPos);
	float4 clipPos =  mul(ProjectionMatrix, viewPos);
	v2p.position = clipPos;
	v2p.color = input.color * ModelColor;
	v2p.uv = input.uv;
    float4 worldNormal = mul(ModelMatrix, float4(input.normal, 0));
    v2p.normal = worldNormal.xyz;
	return v2p;
}

float4 PixelMain(v2p_t input) : SV_Target0
{
	float4 sampledColor = diffuseTexture.Sample(diffuseSampler, input.uv); 
	float4 combinedColor = float4(input.color * sampledColor);
	float sunlight = saturate(dot(normalize(input.normal), -sunDirection));
	float sunshine = sunIntensity * sunlight;
	float light = sunshine + ambientIntensity;
	float4 finalColor = float4(light, light, light, 1.0) * combinedColor;
	clip(finalColor.a - 0.5f);
    return finalColor;
}