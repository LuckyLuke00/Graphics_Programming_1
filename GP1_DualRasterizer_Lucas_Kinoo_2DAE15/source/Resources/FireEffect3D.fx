float4x4 gWorldViewProj : WorldViewProjection;

Texture2D gDiffuseMap : DiffuseMap;

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
	float4 Position	: SV_POSITION;
	float2 UV		: TEXCOORD;
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
RasterizerState gRasterizerState
{
	CullMode = none;
	FrontCounterClockwise = false; // Default
};

//--------------------------------------------------
// Blend State
//--------------------------------------------------
BlendState gBlendState
{
	BlendEnable[0] = true;
	SrcBlend = src_alpha;
	DestBlend = inv_src_alpha;
	BlendOp = add;
	SrcBlendAlpha = zero;
	DestBlendAlpha = zero;
	BlendOpAlpha = add;
	RenderTargetWriteMask[0] = 0x0F;
};
//--------------------------------------------------
// Depth Stencil State
//--------------------------------------------------
DepthStencilState gDepthStencilState
{
	DepthEnable = true;
	DepthWriteMask = zero;
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
	output.UV = input.UV;

	return output;
};

//--------------------------------------------------
// Pixel Shaders
//--------------------------------------------------
float4 PS_Point(VS_OUTPUT input) : SV_TARGET
{
	return gDiffuseMap.Sample(samPoint, input.UV);
}

float4 PS_Linear(VS_OUTPUT input) : SV_TARGET
{
	return gDiffuseMap.Sample(samLinear, input.UV);
}

float4 PS_Anisotropic(VS_OUTPUT input) : SV_TARGET
{
	return gDiffuseMap.Sample(samAnisotropic, input.UV);
}

//--------------------------------------------------
// Techniques
//--------------------------------------------------
technique11 Point
{
	pass P0
	{
		SetRasterizerState(gRasterizerState);
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
		SetRasterizerState(gRasterizerState);
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
		SetRasterizerState(gRasterizerState);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(.0f, .0f, .0f, .0f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Anisotropic()));
	}
}
