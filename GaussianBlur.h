#pragma once
#include <stdint.h>
#include "ColorBuffer.h"
#include "UploadBuffer.h"
#include "PipelineState.h"
#include "RootSignature.h"
#include <Windows.h>
#include <d3d12.h>

#include <memory>
#include <vector>
#include "Mymath.h"
class GaussianBlur
{
public:

	struct VertexData {
		Vector4 pos;
		Vector2 uv;
	};

	GaussianBlur();
	~GaussianBlur();

	static uint32_t gbInstanceCount;
	static void StaticInitialize();
	void CreateMesh();

	void Initialize(ColorBuffer* originalTexture);
	void Render(CommandContext& commandContext);
	void UpdateWeightTable(float blurPower);

	ColorBuffer& GetResult() { return verticalBlurTexture_; }
private:
	static void InitializeGraphicsPipeline();
private:
	static DirectXCommon* sDirectXCommon;
	static ID3D12GraphicsCommandList* sCommandList;
	static std::unique_ptr<RootSignature> sRootSignature;
	static std::unique_ptr<PipelineState> sHorizontalBlurPipelineState;
	static std::unique_ptr<PipelineState> sVerticalBlurPipelineState;

	D3D12_VERTEX_BUFFER_VIEW vbView_{};
	D3D12_INDEX_BUFFER_VIEW ibView_{};
	std::vector<VertexData> vertices_;
	std::vector<uint16_t> indices_;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertBuff_;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBuff_;

	static const uint32_t kNumWeights = 8;

	GaussianBlur(const GaussianBlur&) = delete;
	GaussianBlur& operator=(const GaussianBlur&) = delete;


	ColorBuffer* originalTexture_ = nullptr;
	ColorBuffer horizontalBlurTexture_;
	ColorBuffer verticalBlurTexture_;
	UploadBuffer constantBuffer_;
	float weights_[kNumWeights]{};

	PipelineState horizontalBlurPSO;
	PipelineState verticalBlurPSO;

};

