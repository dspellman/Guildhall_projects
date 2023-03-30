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
};

Texture2D diffuseTexture : register(t0);
SamplerState diffuseSampler : register(s0);

float4 OrthoXform(float3 pt)
{
float4 xpt = (pt, 1);
return mul(ProjectionMatrix, xpt);
}

v2p_t VertexMain(vs_input_t input)
{
	v2p_t v2p;
	float4 xpt = OrthoXform(input.localPosition);
	v2p.position = xpt;
	v2p.position = (input.localPosition, 1);
	v2p.color = input.color;
	v2p.uv = input.uv;
	return v2p;
}

float4 PixelMain(v2p_t input) : SV_Target0
{
	float4 sampledColor = diffuseTexture.Sample(diffuseSampler, input.uv); 
	return float4(input.color * sampledColor);
}

