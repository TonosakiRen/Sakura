#pragma once
#include "GaussianBlur.h"

class CommandContext;

class Bloom
{
public:

    struct VertexData {
        Vector4 pos;
        Vector2 uv;
    };

    static const uint32_t kMaxLevel = 4;

    void Initialize(ColorBuffer* originalTexture);
    void CreateMesh();
    void Render(CommandContext& commandContext, uint32_t level = kMaxLevel);

    void SetThreshold(float threshold) { threshold_ = threshold; }
    void SetKnee(float knee) { knee_ = knee; }

    ColorBuffer& GetResult() { return *originalTexture_; }

    ColorBuffer& GetLuminanceTexture() { return luminanceTexture_; }

private:
    static DirectXCommon* sDirectXCommon;
    RootSignature rootSignature_;
    PipelineState luminacePipelineState_;
    PipelineState additivePipelineState_;

    ColorBuffer* originalTexture_;
    ColorBuffer luminanceTexture_;
    GaussianBlur gaussianBlurs_[kMaxLevel];

    D3D12_VERTEX_BUFFER_VIEW vbView_{};
    D3D12_INDEX_BUFFER_VIEW ibView_{};
    std::vector<VertexData> vertices_;
    std::vector<uint16_t> indices_;
    Microsoft::WRL::ComPtr<ID3D12Resource> vertBuff_;
    Microsoft::WRL::ComPtr<ID3D12Resource> indexBuff_;

    float threshold_ = 0.4f;
    float knee_ = 0.3f;
    float intensity_ = 2.0f;
};

