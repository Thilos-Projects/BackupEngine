struct instance
{
    float4 rect;
    matrix mvp;
};

cbuffer cbChangesPerFrame : register(b0)
{
    instance i;
};

Texture2D colorMap : register(t0);
SamplerState colorSampler : register(s0);

struct VS_Input
{
    float4 pos  : POSITION;
    float2 tex0 : TEXCOORD0;
};

struct PS_Input
{
    float4 pos  : SV_POSITION;
    float2 tex0 : TEXCOORD0;
};

PS_Input VS_Main(VS_Input vertex)
{
    PS_Input vsOut = (PS_Input)0;
    
    vsOut.pos = mul(vertex.pos, i.mvp);
    vsOut.tex0 = float2(vertex.tex0.x * i.rect.x + i.rect.z, vertex.tex0.y * i.rect.y + i.rect.w);

    return vsOut;
}

float4 PS_Main(PS_Input frag) : SV_TARGET
{
    return colorMap.Sample(colorSampler, frag.tex0);
}
