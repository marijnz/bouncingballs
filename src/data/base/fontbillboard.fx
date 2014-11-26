Texture2D diffuse;
SamplerState samNearest
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Clamp;
    AddressV = Clamp;
};

cbuffer cbNeverChanges
{
    matrix View;
};

cbuffer cbChangeOnResize
{
	float2 targetSize;
    matrix Projection;
};

cbuffer cbPerDrawCall
{
	float3 position;
	float4 color;
};

struct VS_INPUT
{
    float2 Pos : POSITION;
    float2 Tex : TEXCOORD;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    float4 pos = mul( float4(position, 1.0f), View );
    float distance = length(pos) * 20;
    pos = mul( pos, Projection );
    pos /= pos.w;

	output.Pos.x = (input.Pos.x) * (1.0f / distance)  + pos.x;
	output.Pos.y = (-input.Pos.y) * (1.0f / distance) + pos.y;
	output.Pos.z = pos.z;
	output.Pos.w = 1.0f;
	output.Tex = input.Tex;

    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{
    return float4(1.0f, 1.0f, 1.0f, diffuse.Sample( samNearest, input.Tex ).r) * color;
    return color;
}

 
//--------------------------------------------------------------------------------------

RasterizerState rs { CullMode = None; };

BlendState bs
{ 
    BlendEnable[0] = True;
	SrcBlend[0] = SRC_ALPHA;
	DestBlend[0] = INV_SRC_ALPHA;
};

technique10 Render
{
    pass P0
    {
		SetRasterizerState(rs);
		SetBlendState(bs, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF);
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS() ) );
    }
}