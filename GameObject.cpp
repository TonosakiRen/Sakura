#include "GameObject.h"

GameObject* GameObject::Create(const std::string name, ViewProjection* viewProjection, DirectionalLight* directionalLight) {
	GameObject* object3d = new GameObject();
	assert(object3d);

	object3d->Initialize(name, viewProjection, directionalLight);

	return object3d;
}


void GameObject::Initialize(const std::string name, ViewProjection* viewProjection, DirectionalLight* directionalLight)
{
	loadObj_ = true;
	model_.Initialize(name);
	worldTransform_.Initialize();
	material_.Initialize();
	SetViewProjection(viewProjection);
	SetDirectionalLight(directionalLight);
}

void GameObject::Initialize(ViewProjection* viewProjection, DirectionalLight* directionalLight)
{
	model_.Initialize();
	worldTransform_.Initialize();
	material_.Initialize();
	SetViewProjection(viewProjection);
	SetDirectionalLight(directionalLight);
}

void GameObject::Update()
{
	worldTransform_.UpdateMatrix();
}

void GameObject::Draw(Vector4 color, uint32_t textureHandle)
{
	if (loadObj_ == true) {
		material_.color_ = color;
		material_.UpdateMaterial();
		model_.Draw(worldTransform_, *viewProjection_, *directionalLight_, material_);
	}
	else {
		material_.color_ = color;
		material_.UpdateMaterial();
		model_.Draw(worldTransform_, *viewProjection_, *directionalLight_, material_, textureHandle);
	}

}

void GameObject::Draw(const WorldTransform& worldTransform, Vector4 color)
{
	material_.color_ = color;
	material_.UpdateMaterial();
	model_.Draw(worldTransform, *viewProjection_, *directionalLight_, material_, 0);
}

void GameObject::UpdateMaterial(Vector4 color)
{
	material_.color_ = color;
	material_.UpdateMaterial();
}