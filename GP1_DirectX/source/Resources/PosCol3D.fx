float4x4 gWorldViewProj : WorldViewProjection;
Texture2D gDiffuseMap : DiffuseMap;

//--------------------------------------------------
//  Input/Output Structs
//--------------------------------------------------
struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Color : COLOR;
    float2 UV : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float3 Color : COLOR0;
    float2 UV : TEXCOORD0;
};

//--------------------------------------------------
//  Samplers
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
//  Vertex Shader
//--------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;

    output.Position = mul(float4(input.Position, 1.f), gWorldViewProj);
    output.Color = input.Color;
    output.UV = input.UV;

    return output;
}

//--------------------------------------------------
//  Pixel Shader
//--------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_TARGET
{
    return gDiffuseMap.Sample(samPoint, input.UV) * float4(input.Color, 1.f);
}

// Point Sampling
float4 PSPoint(VS_OUTPUT input) : SV_TARGET
{
    return gDiffuseMap.Sample(samPoint, input.UV) * float4(input.Color, 1.f);
}

// Linear Sampling
float4 PSLinear(VS_OUTPUT input) : SV_TARGET
{
    return gDiffuseMap.Sample(samLinear, input.UV) * float4(input.Color, 1.f);
}

// Anisotropic Sampling
float4 PSAnisotropic(VS_OUTPUT input) : SV_TARGET
{
    return gDiffuseMap.Sample(samAnisotropic, input.UV) * float4(input.Color, 1.f);
}

//--------------------------------------------------
//  Technique
//--------------------------------------------------
technique11 Render
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS() ) );
    }
}

technique11 RenderPoint
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PSPoint() ) );
    }
}

technique11 RenderLinear
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PSLinear() ) );
    }
}

technique11 RenderAnisotropic
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PSAnisotropic() ) );
    }
}