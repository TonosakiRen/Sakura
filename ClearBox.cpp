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
	ImGui::Begin("ClearBox");
	ImGui::DragFloat3("pos", &worldTransform_.translation_.x);
	ImGui::End();
#endif // _DEBUG

	worldTransform_.UpdateMatrix();
}

void ClearBox::Collision(Collider& otherCollider) {
	Vector3 puchBackVector;
	if (collider_.Collision(otherCollider, puchBackVector)) {
		puchBackVector = puchBackVector * NormalizeMakeRotateMatrix(Inverse(worldTransform_.matWorld_));
		worldTransform_.translation_ += puchBackVector;
		worldTransform_.UpdateMatrix();
	}
}

bool ClearBox::IsHitCollision(Collider& otherCollider) {
	if (collider_.Collision(otherCollider)) {
		return true;
	}
	return false;
}

void ClearBox::Draw() {
	GameObject::Draw();
}
