#pragma once
#include <DirectXMath.h>
#include <d3d12.h>
#include <wrl.h>
#include "Mymath.h"
#include "GPUResource.h"

struct ConstBufferDataDirectionalLight {
	Vector4 color;     
	Vector3 direction; 
	float intensity;  
	int32_t enableLighting;
};

class DirectionalLight
{
public:
	void Initialize();
	void UpdateDirectionalLight();

	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const {
		return constBuff_.GetGPUVirtualAddress();
	}
public:
	Vector4 color_ = { 1.0f, 1.0f, 1.0f,1.0f };
	Vector3 direction_ = { 1.0f, -1.0f, 1.0f };
	float intensity_ = 1.0f;
private:
	void CreateConstBuffer();
	void Map();
private:
	GPUResource constBuff_;
	ConstBufferDataDirectionalLight* constMap = nullptr;
};

