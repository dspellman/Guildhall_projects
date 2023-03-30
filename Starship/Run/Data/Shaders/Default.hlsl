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
	float OrthoMinX;
	float OrthoMinY;
	float OrthoMinZ;
	float OrthoMaxX;
	float OrthoMaxY;
	float OrthoMaxZ;
	float padding1;
	float padding2;
};

Texture2D diffuseTexture : register(t0);
SamplerState diffuseSampler : register(s0);

float3 OrthoXform(float3 pt)
{
float3 xpt = pt;
// x [-1,1]
xpt[0] *= 2;
xpt[0] -= OrthoMinX;
xpt[0] -= OrthoMaxX;
xpt[0] /= (OrthoMaxX - OrthoMinX);
// y [-1,1]
xpt[1] *= 2;
xpt[1] -= OrthoMinY;
xpt[1] -= OrthoMaxY;
xpt[1] /= (OrthoMaxY - OrthoMinY);
// z [0,1]
xpt[2] -= OrthoMinZ;
xpt[2] /= (OrthoMaxZ - OrthoMinZ);
return xpt;
}

v2p_t VertexMain(vs_input_t input)
{
	v2p_t v2p;
	float3 xpt = OrthoXform(input.localPosition);
	v2p.position = float4(xpt, 1);
	v2p.color = input.color;
	v2p.uv = input.uv;
	return v2p;
}

float4 PixelMain(v2p_t input) : SV_Target0
{
	float4 sampledColor = diffuseTexture.Sample(diffuseSampler, input.uv); 
	return float4(input.color * sampledColor);
}

