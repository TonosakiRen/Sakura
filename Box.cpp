#include"Box.h"

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

}

void Box::Update() {
	//コリジョン処理を無効化
	isAlreadyCollision_ = false;

	//重力をベクトルに追加
	Vector3 move = gravity_;

	//回転量に合わせて重力ベクトル変更
	move = move * NormalizeMakeRotateMatrix(Inverse(worldTransform_.matWorld_));

	//加算して移動
	worldTransform_.translation_ += move;

	//行列更新
	worldTransform_.UpdateMatrix();
}

void Box::Draw() {
	// map中心点描画
	GameObject::Draw(worldTransform_);
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
