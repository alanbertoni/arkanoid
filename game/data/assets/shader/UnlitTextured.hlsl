
Texture2D txDiffuse : register( t0 );
SamplerState samLinear : register( s0 );

cbuffer cbChangesEveryObject
{
	matrix World;
	float4 TextureTransf;
};

cbuffer cbChangeOnResize
{
    matrix ViewProjection;
};

cbuffer cbMaterial
{
	float4 vDiffuseColor;
};

struct VS_INPUT
{
    float3 Pos : POSITION;
    float2 Tex : TEXCOORD0;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
	float4 worldPos = float4(input.Pos, 1.0f);
    output.Pos = mul( worldPos, World );
    output.Pos = mul( output.Pos, ViewProjection );
    output.Tex = float2((input.Tex.x * TextureTransf.z) + TextureTransf.x, (input.Tex.y * TextureTransf.w) + TextureTransf.y);
    return output;
}

float4 PS( PS_INPUT input) : SV_Target
{
    return txDiffuse.Sample( samLinear, input.Tex ) * vDiffuseColor;
}
