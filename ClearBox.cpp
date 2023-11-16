#include "ClearBox.h"
#include"ImGuiManager.h"

void ClearBox::Initialize(const std::string name, ViewProjection* viewProjection, DirectionalLight* directionalLight, WorldTransform gWorld) {
	GameObject::Initialize(name, viewProjection, directionalLight);

	//設定したワールドをコピー
	worldTransform_ = gWorld;

	collider_.Initialize(&worldTransform_, name, viewProjection, directionalLight);


}

void ClearBox::Update() {

#ifdef _DEBUG
	
#endif // _DEBUG


}