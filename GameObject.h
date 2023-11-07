#pragma once
#include "Model.h"
#include "ViewProjection.h"
#include "Material.h"
#include "WorldTransform.h"
#include "DirectionalLight.h"
#include "ViewProjection.h"
#include "DirectionalLight.h"
class GameObject
{
public:
	static GameObject* Create(const std::string name, ViewProjection* viewProjection, DirectionalLight* directionalLight);
	void Initialize(const std::string name, ViewProjection* viewProjection, DirectionalLight* directionalLight);
	//modelを使わないInitialize
	void Initialize(ViewProjection* viewProjection, DirectionalLight* directionalLight);
	void Update();
	void UpdateMaterial(Vector4 color = { 1.0f,1.0f,1.0f,1.0f });
	void Draw(Vector4 color = { 1.0f,1.0f,1.0f,1.0f }, uint32_t textureHandle = 0);
	void Draw(const WorldTransform& worldTransform, Vector4 color = { 1.0f,1.0f,1.0f,1.0f });

	void SetViewProjection(ViewProjection* viewProjection) {
		viewProjection_ = viewProjection;
	}
	void SetDirectionalLight(DirectionalLight* directionalLight) {
		directionalLight_ = directionalLight;
	}
	void SetEnableLighting(bool enableLighting) {
		material_.enableLighting_ = enableLighting;
	}
	void SetParent(WorldTransform* parent) {
		worldTransform_.SetParent(parent);
	}
	void SetScale(Vector3 scale) {
		worldTransform_.scale_ = scale;
	}
	void SetRotation(Vector3 rotation) {
		worldTransform_.rotation_ = rotation;
	}
	void SetPosition(Vector3 position) {
		worldTransform_.translation_ = position;
	}
	WorldTransform* GetWorldTransform() {
		return &worldTransform_;
	}
protected:
	WorldTransform worldTransform_;
	Material material_;
	Model model_;
	ViewProjection* viewProjection_;
	DirectionalLight* directionalLight_;
private:
	bool loadObj_ = false;
};