#pragma once
#include <DirectXMath.h>
#include <d3d12.h>
#include <wrl.h>
#include "Mymath.h"

struct ConstBufferDataWorldTransform {
	Matrix4x4 matWorld;
};

class WorldTransform
{
public:
	//bufferに送る場合の初期化
	void Initialize();
	void UpdateMatrix();

	void ConstUpdate() {
		if (constMap) {
			constMap->matWorld = matWorld_;
		}
	}

	void SetParent(WorldTransform* parent) {
		parent_ = parent;
	}

	void SetLocalParent(WorldTransform* parent) {
		if (parent != parent_) {
			if (parent) {
				Matrix4x4 localMatrix = matWorld_ * Inverse(parent->matWorld_);
				translation_ = MakeTranslation(localMatrix);
				if (isRotateParent_ == true) {
					rotation_ = MakeEulerAngle(NormalizeMakeRotateMatrix(localMatrix));
				}
				if (isScaleParent_ == true) {
					scale_ = MakeScale(localMatrix);
				}

				parent_ = parent;
			}
			else {
				translation_ = MakeTranslation(matWorld_);
				if (isRotateParent_ == true) {
					rotation_ = MakeEulerAngle(NormalizeMakeRotateMatrix(matWorld_));
				}
				if (isScaleParent_ == true) {
					scale_ = MakeScale(matWorld_);
				}
				parent_ = parent;
			}
		}

	}
	void SetIsScaleParent(bool isScaleParent) {
		isScaleParent_ = isScaleParent;
	}
	void SetIsRotateParent(bool isRotateParent) {
		isRotateParent_ = isRotateParent;
	}
	WorldTransform* GetParent() {
		return parent_;
	}
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const {
		return constBuff_->GetGPUVirtualAddress();
	}
public:
	Vector3 scale_ = { 1.0f,1.0f,1.0f };
	Vector3 rotation_ = { 0.0f,0.0f,0.0f };
	Vector3 translation_ = { 0.0f,0.0f,0.0f };
	Matrix4x4 matWorld_;
private:
	void CreateConstBuffer();
	void Map();
private:
	WorldTransform* parent_ = nullptr;
	bool isScaleParent_ = true;
	bool isRotateParent_ = true;

	Microsoft::WRL::ComPtr<ID3D12Resource> constBuff_;
	ConstBufferDataWorldTransform* constMap = nullptr;
};

