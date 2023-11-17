#include"Box.h"

#include"ImGuiManager.h"

#include"Map.h"

void Box::Initialize(const std::string name, ViewProjection* viewProjection, DirectionalLight* directionalLight,
	const WorldTransform& world, const int managementNum) {
	//worldとか初期化
	GameObject::Initialize(name, viewProjection, directionalLight);
	
	//設定したワールド渡し
	worldTransform_ = world;

	//管理番号登録
	managementNum_ = managementNum;

	//コライダー初期化
	collider_ = std::make_unique<Collider>();
	collider_->Initialize(&worldTransform_, "blockTile", viewProjection, directionalLight);

	underCollider_ = std::make_unique<Collider>();
	underCollider_->Initialize(&worldTransform_, "blockTile", viewProjection, directionalLight);

}

void Box::Update() {

	ImGui::Begin("box");
	ImGui::Text("%d", state_);
	ImGui::DragFloat3("colloder", &collider_->worldTransform_.translation_.x);
	ImGui::DragFloat3("underColloder", &underCollider_->worldTransform_.translation_.x);
	ImGui::End();

	//コリジョン処理を行ったというフラグの無効化
	isAlreadyCollision_ = false;

	isBuried_ = false;

	switch (state_) {
	case kFall:

		if (!Map::isRotating) {
			//重力をベクトルに追加
			Vector3 move = gravity_;

			//回転量に合わせて重力ベクトル変更
			move = move * NormalizeMakeRotateMatrix(Inverse(worldTransform_.matWorld_));

			//加算して移動
			worldTransform_.translation_ += move;
		}
		break;
	case kStay:
		break;
	default:
		break;
	}

	

	//行列更新
	worldTransform_.UpdateMatrix();


	Vector3 offset = { 0.0f,-2.0f,0.0f };

	offset=offset* NormalizeMakeRotateMatrix(Inverse(worldTransform_.matWorld_));

	underCollider_->worldTransform_.translation_ = offset;

	underCollider_->AdjustmentScale();

}

void Box::Draw() {
	// map中心点描画
	GameObject::Draw(worldTransform_);

	underCollider_->Draw();
}

void Box::Collision(Collider& otherCollider) {

	//押し戻し処理
	Vector3 puchBackVector;
	if (collider_->Collision(otherCollider, puchBackVector)) {
		puchBackVector = puchBackVector * NormalizeMakeRotateMatrix(Inverse(worldTransform_.matWorld_));
		worldTransform_.translation_ += puchBackVector;
		worldTransform_.UpdateMatrix();
		
	}

}

bool Box::IsCollision(Collider& otherCollider) {
	Vector3 puchBackVector;
	if (collider_->Collision(otherCollider, puchBackVector)) {
		return true;
	}
	return false;
}

void Box::CollisionUnderCollider(Collider& other) {
	if (underCollider_->Collision(other)) {
		//止まってる状態
		isBuried_ = true;
		return;
	}
	
}

bool Box::IsCollisionRecurrence(Collider& other) {
	Vector3 puchBackVector;
	if (collider_->Collision(other, puchBackVector)) {
		//ｘ軸の押し出し量がゼロではないのなら
		if (puchBackVector.x != 0) {
			return true;
		}
	}
	return false;
}

void Box::StateChange() {

	if (isBuried_) {
		state_ = kStay;
	}
	else {
		state_ = kFall;
	}
}
