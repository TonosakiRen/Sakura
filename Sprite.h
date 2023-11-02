#pragma once

#include <Windows.h>
#include <d3d12.h>
#include <string>
#include <wrl.h>
#include <memory>
#include "Mymath.h"
#include "PipelineState.h"
#include "GPUResource.h"
#include "PipelineState.h"
#include "RootSignature.h"

class DirectXCommon;
class Sprite {
public:
	struct VertexData {
		Vector3 pos;
		Vector2 uv;
	};

	struct ConstBufferData {
		Vector4 color; 
		Matrix4x4 mat; 
	};

	static void StaticInitialize();
	static void PreDraw(ID3D12GraphicsCommandList* cmdList);
	static void PostDraw();
	static Sprite* Create(uint32_t textureHandle, Vector2 position, Vector4 color = { 1, 1, 1, 1 }, Vector2 anchorpoint = { 0.5f, 0.5f }, bool isFlipX = false, bool isFlipY = false);

	Sprite();
	Sprite(uint32_t textureHandle, Vector2 position, Vector2 size, Vector4 color, Vector2 anchorpoint, bool isFlipX, bool isFlipY);
	bool Initialize();

	void SetTextureHandle(uint32_t textureHandle);
	uint32_t GetTextureHandle() { return textureHandle_; }

	void Draw();
public:

	float rotation_ = 0.0f;
	Vector2 position_{};
	Vector2 size_ = { 100.0f, 100.0f };
	Vector2 anchorPoint_ = { 0.0f, 0.0f };
	Vector4 color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
	bool isFlipX_ = false;
	bool isFlipY_ = false;
	Vector2 texBase_ = { 0, 0 };
	Vector2 texSize_ = { 100.0f, 100.0f };

private:
	static const int kVertNum = 4;
	static DirectXCommon* sDirectXCommon;
	static UINT sDescriptorHandleIncrementSize;
	static ID3D12GraphicsCommandList* sCommandList;
	static std::unique_ptr<RootSignature> sRootSignature;
	static std::unique_ptr<PipelineState> sPipelineState;
	static Matrix4x4 sMatProjection;

private: 
	Microsoft::WRL::ComPtr<ID3D12Resource> vertBuff_;
	GPUResource constBuff_;
	VertexData* vertMap = nullptr;
	ConstBufferData* constMap = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vbView_{};
	UINT textureHandle_ = 0;
	Matrix4x4 matWorld_{};
	D3D12_RESOURCE_DESC resourceDesc_;

private:
	void TransferVertices();
};
