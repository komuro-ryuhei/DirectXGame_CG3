#include "Object3d.hlsli"

struct Material
{
    float4 color;
    int enableLighting;
    float4x4 uvTransform;
    float shininess;
};

struct DirectionalLight
{
    float4 color; // ライトの色
    float3 direction; // ライトの向き
    float intensity; // 輝度
};

struct Camera
{
    float3 worldPosition;
};

ConstantBuffer<Material> gMaterial : register(b0);

ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);

ConstantBuffer<Camera> gCamera : register(b2);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = gMaterial.color;
    
    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    
    output.color = gMaterial.color * textureColor;
    
    float3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
    
    if (gMaterial.enableLighting != 0)
    {
        float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
        float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
        
        float3 reflectLight = reflect(gDirectionalLight.direction, normalize(input.normal));
        
        float3 halfVector = normalize(-gDirectionalLight.direction + toEye);
        
        float NDotH = dot(normalize(input.normal), halfVector);
        
        float RdotE = dot(reflectLight, toEye);
        
        float specularPow = pow(saturate(NDotH), gMaterial.shininess);
        
        // 拡散反射
        float3 diffuse = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
        // 鏡面反射
        float3 specular = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * float3(1.0f, 1.0f, 1.0f);
        // 拡散反射 + 鏡面反射
        output.color.rgb = diffuse + specular;
        // アルファは今まで通り
        output.color.a = gMaterial.color.a * textureColor.a;
        
        //float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
        //float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
        ////float cos = saturate(dot(normalize(input.normal), -gDirectionalLight.direction));
        //output.color.rgb = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
        //output.color.a = gMaterial.color.a * textureColor.a;

    }
    else
    {
        output.color = gMaterial.color * textureColor;
    }
    
    // textureのα値が0の時にPisxelを棄却
    if (textureColor.a == 0.0)
    {
        discard;
    }
    
    // textureのα値が0.5以下の時にPisxelを棄却
    if (textureColor.a <= 0.5)
    {
        discard;
    }
    
    // output.colorのα値が0の時にPisxelを棄却
    if (output.color.a == 0.0)
    {
        discard;
    }
    
    return output;
}