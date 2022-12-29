float gPI = 3.14159265358979323846f;
float gLightIntensity = 7.f;
float gShininess = 25.f;

float3 gLightDirection = normalize(float3(.577f, -.577f, .577f));

float4x4 gWorldViewProj : WorldViewProjection;
float4x4 gWorld : World;
float4x4 gViewInverse : ViewInverse;

Texture2D gDiffuseMap : DiffuseMap;
Texture2D gNormalMap : NormalMap;
Texture2D gSpecularMap : SpecularMap;
Texture2D gGlossinessMap : GlossinessMap;

//--------------------------------------------------
//  Input/Output Structs
//--------------------------------------------------
struct VS_INPUT
{
	float3 Position : POSITION;
	float3 Normal   : NORMAL;
	float3 Tangent  : TANGENT;
	float2 UV       : TEXCOORD;
};

struct VS_OUTPUT
{
	float4 Position         : SV_POSITION;
	float4 WorldPosition    : WORLD_POSITION;
	float3 Normal           : NORMAL;
	float3 Tangent          : TANGENT;
	float2 UV               : TEXCOORD;
};

//--------------------------------------------------
// Samplers
//--------------------------------------------------
SamplerState samPoint
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Wrap; // or Mirror, Clamp, Border
	AddressV = Wrap; // or Mirror, Clamp, Border
};

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap; // or Mirror, Clamp, Border
	AddressV = Wrap; // or Mirror, Clamp, Border
};

SamplerState samAnisotropic
{
	Filter = ANISOTROPIC;
	AddressU = Wrap; // or Mirror, Clamp, Border
	AddressV = Wrap; // or Mirror, Clamp, Border
	MaxAnisotropy = 16;
};

//--------------------------------------------------
// Rasterizer State
//--------------------------------------------------
// RasterizerState gRasterizerState
// {
// 	CullMode = back;
// 	FrontCounterClockwise = false; // Default
// };

//--------------------------------------------------
// Blend State
//--------------------------------------------------
BlendState gBlendState
{
	BlendEnable[0] = false;
	SrcBlend = one;
	DestBlend = zero;
	BlendOp = add;
	SrcBlendAlpha = one;
	DestBlendAlpha = zero;
	BlendOpAlpha = add;
	RenderTargetWriteMask[0] = 0x0F;
};
//--------------------------------------------------
// Depth Stencil State
//--------------------------------------------------
DepthStencilState gDepthStencilState
{
	// Disabled
	DepthEnable = true;
	DepthWriteMask = all;
	DepthFunc = less;
	StencilEnable = false;

	StencilReadMask = 0xFF;
	StencilWriteMask = 0xFF;

	FrontFaceStencilFunc = always;
	BackFaceStencilFunc = always;

	FrontFaceStencilDepthFail = keep;
	BackFaceStencilDepthFail = keep;

	FrontFaceStencilPass = keep;
	BackFaceStencilPass = keep;

	FrontFaceStencilFail = keep;
	BackFaceStencilFail = keep;
};

//--------------------------------------------------
//  Vertex Shader
//--------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	output.Position = mul(float4(input.Position, 1.f), gWorldViewProj);
	output.WorldPosition = mul(float4(input.Position, 1.f), gWorld);
	output.Normal = mul(normalize(input.Normal), (float3x3)gWorld);
	output.Tangent = mul(normalize(input.Tangent), (float3x3)gWorld);
	output.UV = input.UV;

	return output;
}

//--------------------------------------------------
// BRDF
//--------------------------------------------------
float4 Lambert(float kd, float4 cd)
{
	return cd * kd / gPI;
}

float Phong(float ks, float exp, float3 l, float3 v, float3 n)
{
	return ks * pow(saturate(dot(reflect(l, n), v)), exp);
}

//--------------------------------------------------
// Pixel Shaders
//--------------------------------------------------
float4 PS_Point(VS_OUTPUT input) : SV_TARGET
{
	float3 binormal = cross(input.Normal, input.Tangent);
	float3x3 tangentSpaceAxis = float3x3(input.Tangent, binormal, input.Normal);
	float3 normal = mul(normalize(gNormalMap.Sample(samPoint, input.UV).xyz * 2.f - 1.f), tangentSpaceAxis);

	float4 diffuse = Lambert(1.f, gDiffuseMap.Sample(samPoint, input.UV));

	float3 viewDirection = normalize(input.WorldPosition.xyz - gViewInverse[3].xyz);

	float observedArea = saturate(dot(normal, -gLightDirection));

	float exp = gGlossinessMap.Sample(samPoint, input.UV).r * gShininess;
	float4 specular = gSpecularMap.Sample(samPoint, input.UV) * Phong(1.f, exp, -gLightDirection, viewDirection, normal);

	return  (gLightIntensity * diffuse + specular) * observedArea;
}

float4 PS_Linear(VS_OUTPUT input) : SV_TARGET
{
	float3 binormal = cross(input.Normal, input.Tangent);
	float3x3 tangentSpaceAxis = float3x3(input.Tangent, binormal, input.Normal);
	float3 normal = mul(normalize(gNormalMap.Sample(samLinear, input.UV).xyz * 2.f - 1.f), tangentSpaceAxis);

	float4 diffuse = Lambert(1.f, gDiffuseMap.Sample(samLinear, input.UV));

	float3 viewDirection = normalize(input.WorldPosition.xyz - gViewInverse[3].xyz);

	float observedArea = saturate(dot(normal, -gLightDirection));

	float exp = gGlossinessMap.Sample(samLinear, input.UV).r * gShininess;
	float4 specular = gSpecularMap.Sample(samLinear, input.UV) * Phong(1.f, exp, -gLightDirection, viewDirection, normal);

	return  (gLightIntensity * diffuse + specular) * observedArea;
}

float4 PS_Anisotropic(VS_OUTPUT input) : SV_TARGET
{
	float3 binormal = cross(input.Normal, input.Tangent);
	float3x3 tangentSpaceAxis = float3x3(input.Tangent, binormal, input.Normal);
	float3 normal = mul(normalize(gNormalMap.Sample(samAnisotropic, input.UV).xyz * 2.f - 1.f), tangentSpaceAxis);

	float4 diffuse = Lambert(1.f, gDiffuseMap.Sample(samAnisotropic, input.UV));

	float3 viewDirection = normalize(input.WorldPosition.xyz - gViewInverse[3].xyz);

	float observedArea = saturate(dot(normal, -gLightDirection));

	float exp = gGlossinessMap.Sample(samAnisotropic, input.UV).r * gShininess;
	float4 specular = gSpecularMap.Sample(samAnisotropic, input.UV) * Phong(1.f, exp, -gLightDirection, viewDirection, normal);

	return  (gLightIntensity * diffuse + specular) * observedArea;
}

//--------------------------------------------------
// Techniques
//--------------------------------------------------
technique11 Point
{
	pass P0
	{
		//SetRasterizerState(gRasterizerState);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(.0f, .0f, .0f, .0f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Point()));
	}
}

technique11 Linear
{
	pass P0
	{
		//SetRasterizerState(gRasterizerState);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(.0f, .0f, .0f, .0f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Linear()));
	}
}

technique11 Anisotropic
{
	pass P0
	{
		//SetRasterizerState(gRasterizerState);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(.0f, .0f, .0f, .0f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Anisotropic()));
	}
}
