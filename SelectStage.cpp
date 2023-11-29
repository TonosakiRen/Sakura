#include "SelectStage.h"

void SelectStage::Initialize(const std::string name, ViewProjection* viewProjection, DirectionalLight* directionalLight, const WorldTransform& pWorld,int FlyingStageNumber) {
	//基礎初期化
	GameObject::Initialize(name, viewProjection, directionalLight);
	//座標情報更新
	worldTransform_ = pWorld;
	worldTransform_.translation_.y += 1;
	//コライダー初期化
	collider_.Initialize(&worldTransform_, name, viewProjection, directionalLight);

	//飛ぶステージ番号設定
	flyingStageNum_ = FlyingStageNumber;
}

void SelectStage::Update() {
	GameObject::Update();
	
}

void SelectStage::Draw() {
	GameObject::Draw();
	collider_.Draw();
}

bool SelectStage::IsHitCollision(Collider& otherCollider) {
	if (collider_.Collision(otherCollider)) {
		return true;
	}
	return false;
}

