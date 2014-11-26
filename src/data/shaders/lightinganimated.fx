Texture2D diffuse;
SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

cbuffer cbNeverChanges
{
    matrix View;
};

cbuffer cbChangeOnResize
{
    matrix Projection;
};

cbuffer cbChangesEveryFrame
{
    matrix Model;
    matrix Bones[128];
    uint NumBones;
};

struct VS_INPUT
{
    float3 Pos : POSITION;
	float3 Normal : NORMAL;
	float2 Tex : TEXCOORD;
	uint4 BoneIds : BLENDINDICES;
	float4 BoneWeights : BLENDWEIGHT;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
	float3 Normal : TEXCOORD0;
	float2 Tex : TEXCOORD1;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
	
	float4 pos = mul(float4(input.Pos, 1), Bones[input.BoneIds[0]]) * input.BoneWeights[0];
	pos += mul(float4(input.Pos, 1), Bones[input.BoneIds[1]]) * input.BoneWeights[1];
	pos += mul(float4(input.Pos, 1), Bones[input.BoneIds[2]]) * input.BoneWeights[2];
	pos += mul(float4(input.Pos, 1), Bones[input.BoneIds[3]]) * input.BoneWeights[3];
	
	output.Pos = mul( pos, Model );
	
    output.Pos = mul( output.Pos, View );
    output.Pos = mul( output.Pos, Projection );
	
	
	float4 normal = mul(Bones[input.BoneIds[0]], float4(input.Normal, 0)) * input.BoneWeights[0];
	normal += mul(Bones[input.BoneIds[1]], float4(input.Normal, 0)) * input.BoneWeights[1];
	normal += mul(Bones[input.BoneIds[2]], float4(input.Normal, 0)) * input.BoneWeights[2];
	normal += mul(Bones[input.BoneIds[3]], float4(input.Normal, 0)) * input.BoneWeights[3];
    output.Normal = mul( normal.xyz, Model );
	
	
	output.Tex = input.Tex;
    
    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{
	float3 n = normalize(input.Normal);
	float3 lightDir = normalize(float3(2, 2, 3));

	return float4(diffuse.Sample( samLinear, input.Tex ).rgb, 1.0f) * (saturate( dot( n, lightDir) ) + 0.3);
}


//--------------------------------------------------------------------------------------
RasterizerState rs { CullMode = Front; };
DepthStencilState ds { DepthEnable = true; };

BlendState bs
{ 
    BlendEnable[0] = False;
};

technique10 Render
{
    pass P0
    {
	    SetDepthStencilState(ds, 0);
		SetRasterizerState(rs);
		SetBlendState(bs, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF);
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS() ) );
    }
}

