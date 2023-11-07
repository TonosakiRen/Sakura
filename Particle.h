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
#include "DescriptorHandle.h"
#include "PipelineState.h"
#include "RootSignature.h"

class DirectXCommon;

struct InstancingBufferData {
	Matrix4x4 matWorld;
};

class Particle
{
public:

	const uint32_t kParticleNum;

	enum class RootParameter {
		kWorldTransform,
		kViewProjection,
		kTexture,
		kMaterial,
		parameterNum
	};

	struct VertexData {
		Vector3 pos;
		Vector2 uv;
	};

	static void StaticInitialize();
	static void PreDraw(ID3D12GraphicsCommandList* commandList);
	static void PostDraw();
	static Particle* Create(uint32_t particleNum);

	Particle(uint32_t particleNum);
	void Initialize();
	void Draw(const std::vector<InstancingBufferData>& bufferData, const ViewProjection& viewProjection, const DirectionalLight& directionalLight, const Vector4& color, const uint32_t textureHadle);
	void CreateMesh();

public:
	std::vector<InstancingBufferData> particleDatas_;
	Material material_;

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
	DescriptorHandle srvHandle_;
	std::vector<VertexData> vertices_;
	std::vector<uint16_t> indices_;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertBuff_;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBuff_;
	Microsoft::WRL::ComPtr<ID3D12Resource> instancingBuff_;
	InstancingBufferData* instanceMap = nullptr;
};

