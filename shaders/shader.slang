struct VSInput {
    float3 inPosition;
    float2 inTexCoord;;
};

struct UniformBuffer {
    float4x4 model;
    float4x4 view;
    float4x4 proj;
    float deltaTime;
};
ConstantBuffer<UniformBuffer> ubo;

struct VSOutput {
    float4 pos : SV_Position;
    float2 fragTexCoord;
    float4 color;
    uint isParticle;
};

Sampler2D texture;

struct Particle {
    float2 position;
    float2 velocity;
    float4 color;
};

struct ParticleSSBO {
    Particle particles;
};
StructuredBuffer<ParticleSSBO> particlesIn;
RWStructuredBuffer<ParticleSSBO> particlesOut;

struct PushConstants {
    uint renderParticles;
};
[[vk::push_constant]]
PushConstants push;

[shader("vertex")]
VSOutput vertMain(VSInput input, uint vertexId : SV_VertexID, out float pointSize : SV_PointSize) {
    VSOutput output;

    if (push.renderParticles == 1) {
        Particle particle = particlesOut[vertexId].particles;

        output.pos = float4(particle.position, 0.0, 1.0);
        output.fragTexCoord = float2(0.0);
        output.color = particle.color;
        output.isParticle = 1;

        pointSize = 5.0;
    } else {
        output.pos = mul(ubo.proj, mul(ubo.view, mul(ubo.model, float4(input.inPosition, 1))));
        output.fragTexCoord = input.inTexCoord;
        output.color = float4(0.0);
        output.isParticle = 0;

        pointSize = 1.0;
    }

    return output;
}

[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_Target {
    if (vertIn.isParticle == 1) {
        return vertIn.color;
    }

    return texture.Sample(vertIn.fragTexCoord);
}

[shader("compute")]
[numthreads(256, 1, 1)]
void compMain(uint3 threadId : SV_DispatchThreadID)
{
    uint index = threadId.x;

    Particle p = particlesIn[index].particles;

    float dt = ubo.deltaTime * 0.001;

    float2 pos = p.position;
    float2 vel = p.velocity;

    float2 dir = -pos;

    float dist = length(dir) + 0.001;
    float2 n = dir / dist;

    vel += n * (5.0 / (dist + 0.2)) * dt;

    float2 tangent = float2(-n.y, n.x);
    vel += tangent * 15.0 * dt;

    float seed = frac(sin(index * 91.345) * 47453.5453);

    float angle =
        sin(pos.x * 20.0 + seed * 10.0) +
        cos(pos.y * 20.0 - seed * 5.0);

    vel += float2(cos(angle), sin(angle)) * 8.0 * dt;

    if (dist < 0.15)
    {
        vel += normalize(pos) * 4.0 * dt;
    }

    float speed = length(vel);

    if (speed > 3.0)
    {
        vel = normalize(vel) * 3.0;
    }

    vel *= 0.995;

    pos += vel * dt;

    if (pos.x > 1.1) pos.x = -1.1;
    if (pos.x < -1.1) pos.x = 1.1;

    if (pos.y > 1.1) pos.y = -1.1;
    if (pos.y < -1.1) pos.y = 1.1;

    p.position = pos;
    p.velocity = vel;

    particlesOut[index].particles = p;
}
