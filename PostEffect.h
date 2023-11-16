#pragma once
#include "TextureManager.h"
#include "ViewProjection.h"
#include "WorldTransform.h"
#include <Windows.h>
#include <d3d12.h>
#include "externals/DirectXTex/d3dx12.h"
#include <vector>
#include <wrl.h>
#include "Mymath.h"
#include "DirectionalLight.h"
#include "Material.h"
#include <string>
#include "PipelineState.h"
#include "RootSignature.h"

class DirectXCommon;

class PostEffect
{
public:
	enum class RootParameter {
		Constant,
		kTexture,

		ParameterNum
	};

	struct VertexData {
		Vector4 pos;
		Vector2 uv;
	};

	static void StaticInitialize();
	static void PreDraw(ID3D12GraphicsCommandList* commandList);
	static void PostDraw();
	static PostEffect* Create();

	void Initialize();

	void Draw(DescriptorHandle srvHandle);

	void CreateMesh();

private:
	static void InitializeGraphicsPipeline();
private:
	static DirectXCommon* sDirectXCommon;
	static UINT sDescriptorHandleIncrementSize;
	static ID3D12GraphicsCommandList* sCommandList;
	static std::unique_ptr<RootSignature> sRootSignature;
	static std::unique_ptr<PipelineState> sPipelineState;

	D3D12_VERTEX_BUFFER_VIEW vbView_{};
	D3D12_INDEX_BUFFER_VIEW ibView_{};
	std::vector<VertexData> vertices_;
	std::vector<uint16_t> indices_;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertBuff_;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBuff_;
	uint32_t uvHandle_;
	float constant_ = 1.0f;
};

