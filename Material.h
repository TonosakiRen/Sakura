#pragma once
#include <DirectXMath.h>
#include <d3d12.h>
#include <wrl.h>
#include "Mymath.h"
#include "GPUResource.h"

struct ConstBufferDataMaterial {
	Vector4 color;
	Matrix4x4 uvTransform;
	bool enableLighting;
};


class Material
{
public:
	void Initialize();
	void UpdateMaterial();

	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const {
		return constBuff_.GetGPUVirtualAddress();
	}
public:
	Vector4 color_ = { 1.0f, 1.0f, 1.0f,1.0f };
	Vector3 scale_ = { 1.0f,1.0f,1.0f };
	Vector3 rotation_ = { 0.0f,0.0f,0.0f };
	Vector3 translation_ = { 0.0f,0.0f,0.0f };
	bool enableLighting_ = true;
private:
	void CreateConstBuffer();
	void Map();
private:
	Matrix4x4 uvTransform_;
	GPUResource constBuff_;
	ConstBufferDataMaterial* constMap = nullptr;

};

