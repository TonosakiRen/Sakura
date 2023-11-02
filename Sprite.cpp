#include "Sprite.h"
#include "TextureManager.h"
#include <cassert>
#include <d3dcompiler.h>
#include "externals/DirectXTex/d3dx12.h"
#include "DirectXCommon.h"
#include "externals/DirectXTex/DirectXTex.h"
#include "WinApp.h"

#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;
using namespace Microsoft::WRL;

DirectXCommon* Sprite::sDirectXCommon = nullptr;
UINT Sprite::sDescriptorHandleIncrementSize;
ID3D12GraphicsCommandList* Sprite::sCommandList = nullptr;
std::unique_ptr<RootSignature> Sprite::sRootSignature;
std::unique_ptr<PipelineState> Sprite::sPipelineState;
Matrix4x4 Sprite::sMatProjection;

void Sprite::StaticInitialize() {
	
	sDirectXCommon = DirectXCommon::GetInstance();

	sDescriptorHandleIncrementSize =
		sDirectXCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	HRESULT result = S_FALSE;
	ComPtr<IDxcBlob> vsBlob;    
	ComPtr<IDxcBlob> psBlob;    
	ComPtr<ID3DBlob> errorBlob; 

	vsBlob = sDirectXCommon->CompileShader(L"SpriteVS.hlsl", L"vs_6_0");
	assert(vsBlob != nullptr);

	psBlob = sDirectXCommon->CompileShader(L"SpritePS.hlsl", L"ps_6_0");
	assert(psBlob != nullptr);

	sRootSignature = std::make_unique<RootSignature>();
	sPipelineState = std::make_unique<PipelineState>();

	{

		CD3DX12_DESCRIPTOR_RANGE descRangeSRV;
		descRangeSRV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

		CD3DX12_ROOT_PARAMETER rootparams[2] = {};
		rootparams[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
		rootparams[1].InitAsDescriptorTable(1, &descRangeSRV, D3D12_SHADER_VISIBILITY_ALL);

		CD3DX12_STATIC_SAMPLER_DESC samplerDesc =
			CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR); // s0 レジスタ
		samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

		// ルートシグネチャの設定
		D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.pParameters = rootparams;
		rootSignatureDesc.NumParameters = _countof(rootparams);
		rootSignatureDesc.pStaticSamplers = &samplerDesc;
		rootSignatureDesc.NumStaticSamplers = 1;
		rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		sRootSignature->Create(rootSignatureDesc);

	}

	{

		D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
	  {
	   "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
	   D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	  {
	   "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT,
	   D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		};

		D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline{};
		gpipeline.VS = CD3DX12_SHADER_BYTECODE(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize());
		gpipeline.PS = CD3DX12_SHADER_BYTECODE(psBlob->GetBufferPointer(), psBlob->GetBufferSize());

		gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

		gpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

		gpipeline.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		gpipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;


		D3D12_RENDER_TARGET_BLEND_DESC blenddesc{};
		blenddesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		blenddesc.BlendEnable = true;
		blenddesc.BlendOp = D3D12_BLEND_OP_ADD;
		blenddesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blenddesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

		blenddesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blenddesc.SrcBlendAlpha = D3D12_BLEND_ONE;
		blenddesc.DestBlendAlpha = D3D12_BLEND_ZERO;


		gpipeline.BlendState.RenderTarget[0] = blenddesc;


		gpipeline.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;


		gpipeline.InputLayout.pInputElementDescs = inputLayout;
		gpipeline.InputLayout.NumElements = _countof(inputLayout);


		gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

		gpipeline.NumRenderTargets = 1;
		gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		gpipeline.SampleDesc.Count = 1;

		gpipeline.pRootSignature = *sRootSignature;

		// グラフィックスパイプラインの生成
		sPipelineState->Create(gpipeline);

	}

	sMatProjection = MakeOrthograohicmatrix(0.0f, 0.0f, (float)WinApp::kWindowWidth, (float)WinApp::kWindowHeight,  0.0f, 1.0f);
}

void Sprite::PreDraw(ID3D12GraphicsCommandList* commandList) {
	assert(Sprite::sCommandList == nullptr);

	sCommandList = commandList;

	commandList->SetPipelineState(*sPipelineState);
	commandList->SetGraphicsRootSignature(*sRootSignature);
	sCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

void Sprite::PostDraw() {
	Sprite::sCommandList = nullptr;
}

Sprite* Sprite::Create(uint32_t textureHandle, Vector2 position, Vector4 color, Vector2 anchorpoint, bool isFlipX, bool isFlipY) {
	Vector2 size = { 100.0f, 100.0f };

	const D3D12_RESOURCE_DESC& resDesc = TextureManager::GetInstance()->GetResoureDesc(textureHandle);
	size = { (float)resDesc.Width, (float)resDesc.Height };
	
	Sprite* sprite = new Sprite(textureHandle, position, size, color, anchorpoint, isFlipX, isFlipY);
	if (sprite == nullptr) {
		return nullptr;
	}

	if (!sprite->Initialize()) {
		delete sprite;
		assert(0);
		return nullptr;
	}

	return sprite;
}

Sprite::Sprite() {}

Sprite::Sprite(uint32_t textureHandle, Vector2 position, Vector2 size, Vector4 color, Vector2 anchorpoint,bool isFlipX, bool isFlipY) {
	position_ = position;
	size_ = size;
	anchorPoint_ = anchorpoint;
	matWorld_ = MakeIdentity4x4();
	color_ = color;
	textureHandle_ = textureHandle;
	isFlipX_ = isFlipX;
	isFlipY_ = isFlipY;
	texSize_ = size;
}

bool Sprite::Initialize() {
	assert(sDirectXCommon->GetDevice());

	HRESULT result = S_FALSE;

	resourceDesc_ = TextureManager::GetInstance()->GetResoureDesc(textureHandle_);

	{
		// ヒーププロパティ
		CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		// リソース設定
		CD3DX12_RESOURCE_DESC resourceDesc =
			CD3DX12_RESOURCE_DESC::Buffer(sizeof(VertexData) * kVertNum);

		// 頂点バッファ生成
		result = sDirectXCommon->GetDevice()->CreateCommittedResource(
			&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, IID_PPV_ARGS(&vertBuff_));
		assert(SUCCEEDED(result));

		// 頂点バッファマッピング
		result = vertBuff_->Map(0, nullptr, (void**)&vertMap);
		assert(SUCCEEDED(result));
	}

	// 頂点バッファへのデータ転送
	TransferVertices();

	// 頂点バッファビューの作成
	vbView_.BufferLocation = vertBuff_->GetGPUVirtualAddress();
	vbView_.SizeInBytes = sizeof(VertexData) * 4;
	vbView_.StrideInBytes = sizeof(VertexData);

	{
		// ヒーププロパティ
		CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		// リソース設定
		CD3DX12_RESOURCE_DESC resourceDesc =
			CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferData) + 0xff) & ~0xff);

		// 定数バッファの生成
		result = sDirectXCommon->GetDevice()->CreateCommittedResource(
			&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, IID_PPV_ARGS(constBuff_.GetAddressOf()));
		assert(SUCCEEDED(result));
	}

	// 定数バッファマッピング
	result = constBuff_->Map(0, nullptr, (void**)&constMap);
	assert(SUCCEEDED(result));

	return true;
}

void Sprite::SetTextureHandle(uint32_t textureHandle) {
	textureHandle_ = textureHandle;
	resourceDesc_ = TextureManager::GetInstance()->GetResoureDesc(textureHandle_);
}

void Sprite::Draw() {

	TransferVertices();

	matWorld_ = MakeIdentity4x4();
	matWorld_ *= MakeRotateZMatrix(rotation_);
	matWorld_ *= MakeTranslateMatrix({ position_.x, position_.y, 0.0f });

	constMap->color = color_;
	constMap->mat = matWorld_ * sMatProjection;

	sCommandList->IASetVertexBuffers(0, 1, &vbView_);

	sCommandList->SetGraphicsRootConstantBufferView(0, constBuff_->GetGPUVirtualAddress());

	TextureManager::GetInstance()->SetGraphicsRootDescriptorTable(sCommandList, 1, textureHandle_);
	
	sCommandList->DrawInstanced(4, 1, 0, 0);
}

void Sprite::TransferVertices() {
	HRESULT result = S_FALSE;

	enum { LB, LT, RB, RT };

	float left = (0.0f - anchorPoint_.x) * size_.x;
	float right = (1.0f - anchorPoint_.x) * size_.x;
	float top = (0.0f - anchorPoint_.y) * size_.y;
	float bottom = (1.0f - anchorPoint_.y) * size_.y;
	if (isFlipX_) { 
		left = -left;
		right = -right;
	}

	if (isFlipY_) {
		top = -top;
		bottom = -bottom;
	}

	// 頂点データ
	VertexData vertices[kVertNum];

	vertices[LB].pos = { left, bottom, 0.0f };  
	vertices[LT].pos = { left, top, 0.0f };     
	vertices[RB].pos = { right, bottom, 0.0f }; 
	vertices[RT].pos = { right, top, 0.0f };    

	{
		float tex_left = texBase_.x / resourceDesc_.Width;
		float tex_right = (texBase_.x + texSize_.x) / resourceDesc_.Width;
		float tex_top = texBase_.y / resourceDesc_.Height;
		float tex_bottom = (texBase_.y + texSize_.y) / resourceDesc_.Height;

		vertices[LB].uv = { tex_left, tex_bottom };  
		vertices[LT].uv = { tex_left, tex_top };     
		vertices[RB].uv = { tex_right, tex_bottom }; 
		vertices[RT].uv = { tex_right, tex_top };    
	}

	// 頂点バッファへのデータ転送
	memcpy(vertMap, vertices, sizeof(vertices));
}
