#include "ClearBox.h"
#include"ImGuiManager.h"
#include"Map.h"

void ClearBox::Initialize(const std::string name, ViewProjection* viewProjection, DirectionalLight* directionalLight, WorldTransform gWorld) {
	GameObject::Initialize(name, viewProjection, directionalLight);

	//設定したワールドをコピー
	worldTransform_ = gWorld;

	collider_.Initialize(&worldTransform_, name, viewProjection, directionalLight, portraitScale);

	rectangleState_ = RectangleFacing::kPortrait;
	worldTransform_.translation_.y += 0.5f;
}

void ClearBox::StageInitialize(WorldTransform gWorld,int stageNum)
{
	//設定したワールドをコピー
	worldTransform_ = gWorld;
	worldTransform_.UpdateMatrix();

	switch (stageNum)
	{
	case 0:
		rectangleState_ = RectangleFacing::kPortrait;
		break;
	case 1:
		rectangleState_ = RectangleFacing::kPortrait;
		break;
	case 2:
		rectangleState_ = RectangleFacing::kPortrait;
		break;
	case 3:
		rectangleState_ = RectangleFacing::kPortrait;
		break;
	case 4:
		rectangleState_ = RectangleFacing::kLandscape;
		isRight = false;
		break;
	case 5:
		rectangleState_ = RectangleFacing::kPortrait;
		break;
	case 6:
		rectangleState_ = RectangleFacing::kLandscape;
		isRight = true;
		break;
	case 7:
		rectangleState_ = RectangleFacing::kLandscape;
		isRight = true;
		break;
	case 8:
		rectangleState_ = RectangleFacing::kPortrait;
		break;
	case 9:
		rectangleState_ = RectangleFacing::kLandscape;
		isRight = false;
		break;
	case 10:
		rectangleState_ = RectangleFacing::kLandscape;
		isRight = true;
		break;
	default:
		break;
	}

	if (rectangleState_ == RectangleFacing::kLandscape) {
		//worldTransform_.scale_ = landScapeScale;
		worldTransform_.rotation_.z = Radian(90.0f);
		if (isRight) {
			worldTransform_.translation_.x -= 0.5f;
		}
		else {
			worldTransform_.translation_.x += 0.5f;
		}
	}
	else {
		//worldTransform_.scale_ = portraitScale;
		worldTransform_.translation_.y += 0.5f;
	}

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

	UpdateMatirx();

	if (Map::rotateComplete) {
		if (rectangleState_ != RectangleFacing::kPortrait) {
			rectangleState_ = RectangleFacing::kPortrait;
		}
		else if (rectangleState_ != RectangleFacing::kLandscape) {
			rectangleState_ = RectangleFacing::kLandscape;
		}
	}
	
}

void ClearBox::UpdateMatirx() {
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
	collider_.Draw();
}
