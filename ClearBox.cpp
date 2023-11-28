#include "ClearBox.h"
#include"ImGuiManager.h"
#include"Map.h"

void ClearBox::Initialize(const std::string name, ViewProjection* viewProjection, DirectionalLight* directionalLight, WorldTransform gWorld) {
	GameObject::Initialize(name, viewProjection, directionalLight);

	//設定したワールドをコピー
	worldTransform_ = gWorld;

	collider_.Initialize(&worldTransform_, name, viewProjection, directionalLight);

	if (rectangleState_ == RectangleFacing::kPortrait) {
		worldTransform_.scale_ = portraitScale;
	}
	else {
		worldTransform_.scale_ = landScapeScale;
	}
}

void ClearBox::StageInitialize(WorldTransform gWorld)
{
	//設定したワールドをコピー
	worldTransform_ = gWorld;
	worldTransform_.UpdateMatrix();

}

void ClearBox::Update() {

#ifdef _DEBUG
	ImGui::Begin("ClearBox");
	ImGui::DragFloat3("pos", &worldTransform_.translation_.x);
	switch (rectangleState_) {
	case RectangleFacing::kPortrait:
		ImGui::Text("boxState : Portrait");
		break;
	case RectangleFacing::kLandscape:
		ImGui::Text("boxState : Landscape");
		break;
	default:
		break;
	}
	ImGui::End();
#endif // _DEBUG

	worldTransform_.UpdateMatrix();


	if (Map::rotateComplete) {
		if (rectangleState_ != RectangleFacing::kPortrait) {
			rectangleState_ = RectangleFacing::kPortrait;
		}
		else if (rectangleState_ != RectangleFacing::kLandscape) {
			rectangleState_ = RectangleFacing::kLandscape;
		}
	}
	
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
