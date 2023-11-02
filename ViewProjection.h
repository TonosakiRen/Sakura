#pragma once
#include <DirectXMath.h>
#include <d3d12.h>
#include <wrl.h>
#include "Mymath.h"
#include <numbers>

struct ConstBufferDataViewProjection {
	Matrix4x4 view;
	Matrix4x4 projection;
	Vector3 viewPosition;
};

class ViewProjection {
public:
	void Initialize();
	void DebugMove();
	void UpdateMatrix();
	bool Shake(Vector3 shakeValue, int& frame);

	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const {
		return constBuff_->GetGPUVirtualAddress();
	}

	void SetFarZ(float farZ) {
		farZ_ = farZ;
	}
public:
	Vector3 translation_ = { 0.0f, 0.0f, -10.0f };
	Vector3 target_ = { 0.0f, 0.0f, 0.0f };
	Vector3 shakeValue_ = { 0.0f,0.0f,0.0f };
private:
	void CreateConstBuffer();
	void Map();
private:
	float fovAngleY_ = 45.0f * std::numbers::pi_v <float> / 180.0f;
	float aspectRatio_ = (float)16 / (float)9;
	float nearZ_ = 0.1f;
	float farZ_ = 1000.0f;

	Matrix4x4 matView;
	Matrix4x4 matProjection;

	Microsoft::WRL::ComPtr<ID3D12Resource> constBuff_;
	ConstBufferDataViewProjection* constMap = nullptr;
};


