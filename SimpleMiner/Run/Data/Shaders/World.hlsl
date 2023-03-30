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
	float4 worldPos : WorldPos;
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

cbuffer GameConstants : register(b8)
{
	float4		b_camWorldPos;			// Used for fog thickness calculations, specular lighting, etc.
	float4		b_skyColor;				// Also used as fog color
	float4		b_outdoorLightColor;	// Used for outdoor lighting exposure
	float4		b_indoorLightColor;		// Used for outdoor lighting exposure
	float		b_fogStartDist;			// Fog has zero opacity at or before this distance
	float		b_fogEndDist;			// Fog has maximum opacity at or beyond this distance
	float		b_fogMaxAlpha;			// At and beyond fogEndDist, fog gets this much opacity
	float		b_time;					// time is a handy value in many instances
};

Texture2D diffuseTexture : register(t0);
SamplerState diffuseSampler : register(s0);

v2p_t VertexMain(vs_input_t input)
{
	v2p_t v2p;
	float3 cameraWorldPos = -ViewMatrix._m30_m31_m32;
	float4 localPos = float4(input.localPosition, 1.0);
	float4 worldPos = mul(ModelMatrix, localPos);
	float4 viewPos = mul(ViewMatrix, worldPos);
	float4 clipPos =  mul(ProjectionMatrix, viewPos);
	v2p.position = clipPos;
	v2p.color = input.color * ModelColor;
	v2p.uv = input.uv;
	v2p.worldPos = worldPos;
	return v2p;
}

float3 DiminishingAddComponents( float3 a, float3 b )
{
	   return 1.0f - (1.0f - a) * (1.0f - b);
}

float4 PixelMain(v2p_t input) : SV_Target0
{
	float4 diffuseTexel = diffuseTexture.Sample(diffuseSampler, input.uv); 
	clip( diffuseTexel.a - 0.01 );
	
	// Compute lit pixel color
	float outdoorLightExposure = input.color.r;
	float indoorLightExposure = input.color.g;
	float3 outdoorLight = outdoorLightExposure * b_outdoorLightColor.rgb;
	float3 indoorLight = indoorLightExposure * b_indoorLightColor.rgb;
	float3 diffuseLight = DiminishingAddComponents( outdoorLight, indoorLight );
	float3 diffuseRGB = diffuseLight * diffuseTexel.rgb;
	
	// Compute the fog
	float3 dispCamToPixel = input.worldPos.xyz - b_camWorldPos.xyz;
	float distCamToPixel = length( dispCamToPixel );
	float fogDensity = b_fogMaxAlpha * saturate( (distCamToPixel - b_fogStartDist) / (b_fogEndDist - b_fogStartDist) );
	float3 finalRGB = lerp( diffuseRGB, b_skyColor.rgb, fogDensity );
	float finalAlpha = saturate( diffuseTexel.a + fogDensity ); // fog can add opacity
	float4 finalColor = float4( finalRGB, finalAlpha );
	return finalColor;
}
