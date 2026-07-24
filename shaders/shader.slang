struct VSInput {
    float3 inPosition;
    float2 inTexCoord;
};

struct UniformBuffer {
    float4x4 view;
    float4x4 proj;
    float3 cameraPosition;
    float deltaTime;
};
ConstantBuffer<UniformBuffer> ubo;

struct VSOutput {
    float4 pos : SV_Position;
    float2 fragTexCoord;
    float4 color;
};

struct PushConstants {
    float4x4 model;
};
[[vk::push_constant]]
PushConstants push;

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    VSOutput output;

    output.pos = mul(ubo.proj, mul(ubo.view, mul(push.model, float4(input.inPosition, 1))));
    output.fragTexCoord = input.inTexCoord;
    output.color = float4(1.0);

    return output;
}

[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_Target {
    return vertIn.color;

    // return texture.Sample(vertIn.fragTexCoord);
}
