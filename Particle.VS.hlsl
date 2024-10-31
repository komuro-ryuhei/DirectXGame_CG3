#include "Particle.hlsli"

struct ParticleForGPU
{
    float4x4 WVP;
    float4x4 World;
    float4 color;
};

StructuredBuffer<ParticleForGPU> gParticle : register(t0);

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
    float4 color : COLOR0;
};

VertexShaderOutput main(VertexShaderInput input, uint instancesId : SV_InstanceID)
{
    VertexShaderOutput output;
    output.position = mul(input.position, gParticle[instancesId].WVP);
    output.texcoord = input.texcoord;
    
    output.normal = normalize(mul(input.normal, (float3x3) gParticle[instancesId].World));
    
    output.color = gParticle[instancesId].color;
    
    return output;
}