Texture2D<float4> tex : register(t1);
SamplerState smp : register(s0);

struct Material {
	float32_t4 materialColor; //Materialの色
	float32_t4x4 uvTransfrom;//uvtransform
};
ConstantBuffer<Material> gMaterial  : register(b1);

struct VSOutput {
	float32_t4 pos : SV_POSITION; 
	float32_t2 uv : TEXCOORD;      
};

struct PixelShaderOutput {
	float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VSOutput input) {

	PixelShaderOutput output;
	float32_t4 tranformedUV = mul(float32_t4(input.uv, 0.0f, 1.0f), gMaterial.uvTransfrom);
	float32_t4 texcolor = tex.Sample(smp, tranformedUV.xy);
	output.color = texcolor * gMaterial.materialColor;
	if (output.color.a == 0.0) {
		discard;
	}
	return output;
}