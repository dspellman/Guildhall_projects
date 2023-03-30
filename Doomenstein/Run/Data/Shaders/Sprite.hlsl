struct vs_input_t
{
	float3 localPosition : POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};

struct v2p_t
{
	float4 position : SV_Position;
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
	return v2p;
}

float4 PixelMain(v2p_t input) : SV_Target0
{
	float4 sampledColor = diffuseTexture.Sample(diffuseSampler, input.uv); 
	float4 combined = float4(input.color * sampledColor);
	clip( combined.a - 0.5f );
	return combined;
}