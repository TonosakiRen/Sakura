#include "PostEffect.h"
#include "externals/DirectXTex/DirectXTex.h"
#include <d3dcompiler.h>
#include "DirectXCommon.h"
#include <fstream>
#include <sstream>
#pragma comment(lib, "d3dcompiler.lib")
#include "TextureManager.h"
#include "DWParam.h"

using namespace DirectX;
using namespace Microsoft::WRL;

DirectXCommon* PostEffect::sDirectXCommon = nullptr;
UINT PostEffect::sDescriptorHandleIncrementSize = 0;
ID3D12GraphicsCommandList* PostEffect::sCommandList = nullptr;
std::unique_ptr<RootSignature> PostEffect::sRootSignature;
std::unique_ptr<PipelineState> PostEffect::sPipelineState;

void PostEffect::StaticInitialize() {
	sDirectXCommon = DirectXCommon::GetInstance();
	InitializeGraphicsPipeline();
}

void PostEffect::PreDraw(ID3D12GraphicsCommandList* commandList) {
	assert(PostEffect::sCommandList == nullptr);

	sCommandList = commandList;

	commandList->SetPipelineState(*sPipelineState);
	commandList->SetGraphicsRootSignature(*sRootSignature);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void PostEffect::PostDraw() {
	sCommandList = nullptr;
}

PostEffect* PostEffect::Create() {
	PostEffect* object3d = new PostEffect();
	assert(object3d);

	// 初期化
	object3d->Initialize();

	return object3d;
}

void PostEffect::InitializeGraphicsPipeline() {
	HRESULT result = S_FALSE;
	ComPtr<IDxcBlob> vsBlob;
	ComPtr<IDxcBlob> psBlob;
	ComPtr<ID3DBlob> errorBlob;

	vsBlob = sDirectXCommon->CompileShader(L"PostEffectVS.hlsl", L"vs_6_0");
	assert(vsBlob != nullptr);

	psBlob = sDirectXCommon->CompileShader(L"PostEffectPS.hlsl", L"ps_6_0");
	assert(psBlob != nullptr);

	sRootSignature = std::make_unique<RootSignature>();
	sPipelineState = std::make_unique<PipelineState>();

	{

		// デスクリプタレンジ
		CD3DX12_DESCRIPTOR_RANGE descRangeSRV;
		descRangeSRV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // t0 レジスタ

		// ルートパラメータ
		CD3DX12_ROOT_PARAMETER rootparams[int(RootParameter::ParameterNum)] = {};
		rootparams[int(RootParameter::Constant)].InitAsConstants(1, 0);
		rootparams[int(RootParameter::kTexture)].InitAsDescriptorTable(1, &descRangeSRV, D3D12_SHADER_VISIBILITY_ALL);

		// スタティックサンプラー
		CD3DX12_STATIC_SAMPLER_DESC samplerDesc =
			CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR);

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
			"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
		   D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		  {
		   "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT,
		   D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		};

		// グラフィックスパイプラインの流れを設定
		D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline{};
		gpipeline.VS = CD3DX12_SHADER_BYTECODE(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize());
		gpipeline.PS = CD3DX12_SHADER_BYTECODE(psBlob->GetBufferPointer(), psBlob->GetBufferSize());

		// サンプルマスク
		gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; // 標準設定
		// ラスタライザステート
		gpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		//  デプスステンシルステート
		//gpipeline.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

		// レンダーターゲットのブレンド設定
		D3D12_RENDER_TARGET_BLEND_DESC blenddesc{};
		blenddesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		blenddesc.BlendEnable = true;
		blenddesc.BlendOp = D3D12_BLEND_OP_ADD;
		blenddesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blenddesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

		blenddesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blenddesc.SrcBlendAlpha = D3D12_BLEND_ONE;
		blenddesc.DestBlendAlpha = D3D12_BLEND_ZERO;

		// ブレンドステートの設定
		gpipeline.BlendState.RenderTarget[0] = blenddesc;

		// 深度バッファのフォーマット
		//gpipeline.DSVFormat = nullptr;

		// 頂点レイアウトの設定
		gpipeline.InputLayout.pInputElementDescs = inputLayout;
		gpipeline.InputLayout.NumElements = _countof(inputLayout);

		// 図形の形状設定（三角形）
		gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

		gpipeline.NumRenderTargets = 1;                            // 描画対象は1つ
		gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // 0～255指定のRGBA
		gpipeline.SampleDesc.Count = 1; // 1ピクセルにつき1回サンプリング

		gpipeline.pRootSignature = *sRootSignature;

		// グラフィックスパイプラインの生成
		sPipelineState->Create(gpipeline);
	}
}

void PostEffect::CreateMesh() {
	HRESULT result = S_FALSE;

	vertices_.resize(4);

	//左下
	vertices_[0].pos = { -1.0f,-1.0f,0.0f, 1.0f };
	vertices_[0].uv = { 0.0f,1.0f };
	//左上
	vertices_[1].pos = { -1.0f,1.0f,0.0f, 1.0f };
	vertices_[1].uv = { 0.0f,0.0f };
	//右上
	vertices_[2].pos = { 1.0f,1.0f,0.0f, 1.0f };
	vertices_[2].uv = { 1.0f,0.0f };
	//右下
	vertices_[3].pos = { 1.0f,-1.0f,0.0f, 1.0f };
	vertices_[3].uv = { 1.0f,1.0f };

	// 頂点インデックスの設定
	indices_ = { 0,  1,  2, 0, 2, 3 };

	// 頂点データのサイズ
	UINT sizeVB = static_cast<UINT>(sizeof(VertexData) * vertices_.size());

	{
		// ヒーププロパティ
		CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		// リソース設定
		CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeVB);

		// 頂点バッファ生成
		result = sDirectXCommon->GetDevice()->CreateCommittedResource(
			&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
			IID_PPV_ARGS(&vertBuff_));
		assert(SUCCEEDED(result));
	}

	// 頂点バッファへのデータ転送
	VertexData* vertMap = nullptr;
	result = vertBuff_->Map(0, nullptr, (void**)&vertMap);
	if (SUCCEEDED(result)) {
		std::copy(vertices_.begin(), vertices_.end(), vertMap);
		vertBuff_->Unmap(0, nullptr);
	}

	// 頂点バッファビューの作成
	vbView_.BufferLocation = vertBuff_->GetGPUVirtualAddress();
	vbView_.SizeInBytes = sizeVB;
	vbView_.StrideInBytes = sizeof(vertices_[0]);


	// インデックスデータのサイズ
	UINT sizeIB = static_cast<UINT>(sizeof(uint16_t) * indices_.size());

	{
		// ヒーププロパティ
		CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		// リソース設定
		CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeIB);

		// インデックスバッファ生成
		result = sDirectXCommon->GetDevice()->CreateCommittedResource(
			&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
			IID_PPV_ARGS(&indexBuff_));
		assert(SUCCEEDED(result));
	}

	// インデックスバッファへのデータ転送
	uint16_t* indexMap = nullptr;
	result = indexBuff_->Map(0, nullptr, (void**)&indexMap);
	if (SUCCEEDED(result)) {
		std::copy(indices_.begin(), indices_.end(), indexMap);

		indexBuff_->Unmap(0, nullptr);
	}

	// インデックスバッファビューの作成
	ibView_.BufferLocation = indexBuff_->GetGPUVirtualAddress();
	ibView_.Format = DXGI_FORMAT_R16_UINT;
	ibView_.SizeInBytes = sizeIB;


}

void PostEffect::Initialize() {
	// nullptrチェック
	assert(sDirectXCommon->GetDevice());

	// メッシュ生成
	CreateMesh();
}

void PostEffect::Draw(DescriptorHandle srvHandle) {

	// 頂点バッファの設定
	sCommandList->IASetVertexBuffers(0, 1, &vbView_);

	// インデックスバッファの設定
	sCommandList->IASetIndexBuffer(&ibView_);

	DWParam constant = constant_;

	// CBVをセット（ワールド行列）
	sCommandList->SetGraphicsRoot32BitConstant(static_cast<UINT>(RootParameter::Constant), constant.v.u, 0);

	// SRVをセット
	sCommandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(RootParameter::kTexture), srvHandle);

	/*  TextureManager::GetInstance()->SetGraphicsRootDescriptorTable(sCommandList, 1, 0);*/

	  // 描画コマンド
	sCommandList->DrawIndexedInstanced(static_cast<UINT>(indices_.size()), 1, 0, 0, 0);
}

