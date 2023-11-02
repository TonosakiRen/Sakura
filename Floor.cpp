#include "Floor.h"

void Floor::Initialize(const std::string name, ViewProjection* viewProjection, DirectionalLight* directionalLight)
{
	GameObject::Initialize(name, viewProjection, directionalLight);
	material_.enableLighting_ = false;
	worldTransform_.scale_ = worldTransform_.scale_ * 50.0f;
	collider_.Initialize(&worldTransform_,name, viewProjection, directionalLight,{20.0f, 1.0f, 1.0f});
}

void Floor::Update()
{
	collider_.AdjustmentScale();
	material_.translation_.y += -0.001f;
}

void Floor::Draw() {
	collider_.Draw();
	GameObject::Draw();
}