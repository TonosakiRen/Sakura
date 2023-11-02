
struct WorldTransform {
	float32_t4x4 world;
};
StructuredBuffer<WorldTransform> gWorldTransforms  : register(t0);

struct ViewProjection {
	float32_t4x4 view;
	float32_t4x4 projection;
};
ConstantBuffer<ViewProjection> gViewProjection  : register(b0);

struct VSInput {
	float32_t4 pos : POSITION;
	float32_t2 uv : TEXCOORD;
	uint32_t instanceId : SV_InstanceID;
};

struct VSOutput {
	float32_t4 svpos : SV_POSITION;
	float32_t2 uv : TEXCOORD;
};

VSOutput main(VSInput input) {
	VSOutput output; 
	output.svpos = mul(input.pos, mul(gWorldTransforms[input.instanceId].world, mul(gViewProjection.view, gViewProjection.projection)));
	output.uv = input.uv;
	return output;
}