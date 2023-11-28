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

	if (!isDead_) {

#ifdef _DEBUG
		ImGui::Begin("box");
		ImGui::Text("%d", state_);
		ImGui::DragFloat3("colloder", &collider_->worldTransform_.translation_.x);
		ImGui::DragFloat3("underColloder", &underCollider_->worldTransform_.translation_.x);
		ImGui::End();
#endif // _DEBUG

		//コリジョン処理を行ったというフラグの無効化
		isAlreadyCollision_ = false;

		isBuried_ = false;

		switch (state_) {
		case kFall:

			if (!Map::isRotating) {
				//重力をベクトルに追加
				move_ = gravity_;

				//回転量に合わせて重力ベクトル変更
				move_ = move_ * NormalizeMakeRotateMatrix(Inverse(worldTransform_.matWorld_));

				//加算して移動
				worldTransform_.translation_ += move_;
			}
			break;
		case kStay:
			break;
		default:
			break;
		}



		//行列更新
		worldTransform_.UpdateMatrix();
	}
}

void Box::Draw() {
	if (!isDead_) {
		// map中心点描画
		GameObject::Draw(worldTransform_);
	}
	//underCollider_->Draw();
}

void Box::Collision(Collider& otherCollider,const Vector3& priorityVector) {

	Vector3 rotatedpriotiyVector = priorityVector ;
	//押し戻し処理
	Vector3 puchBackVector;
	if (collider_->Collision(otherCollider, puchBackVector, rotatedpriotiyVector)) {
		
		//誤差
		float ErrorNum = 0.001f;

		//誤差レベルの数字は0にする
		if (puchBackVector.x<ErrorNum && puchBackVector.x>-ErrorNum) {
			puchBackVector.x = 0;
		}
		if (puchBackVector.y<ErrorNum && puchBackVector.y>-ErrorNum) {
			puchBackVector.y = 0;
		}
		if (puchBackVector.z<ErrorNum && puchBackVector.z>-ErrorNum) {
			puchBackVector.z = 0;
		}

		//埋まらずぴったり横の場合
		if (puchBackVector.x == 0 && puchBackVector.y == 0 && puchBackVector.z == 0) {
		}
		else {
			puchBackVector = puchBackVector * NormalizeMakeRotateMatrix(Inverse(worldTransform_.matWorld_));
			worldTransform_.translation_ += puchBackVector;
			worldTransform_.UpdateMatrix();
		}
	}

}

void Box::Collision(Collider& otherCollider) {


	//押し戻し処理
	Vector3 puchBackVector;
	if (collider_->Collision(otherCollider, puchBackVector)) {

		//誤差
		float ErrorNum = 0.001f;

		//誤差レベルの数字は0にする
		if (puchBackVector.x<ErrorNum && puchBackVector.x>-ErrorNum) {
			puchBackVector.x = 0;
		}
		if (puchBackVector.y<ErrorNum && puchBackVector.y>-ErrorNum) {
			puchBackVector.y = 0;
		}
		if (puchBackVector.z<ErrorNum && puchBackVector.z>-ErrorNum) {
			puchBackVector.z = 0;
		}

		//埋まらずぴったり横の場合
		if (puchBackVector.x == 0 && puchBackVector.y == 0 && puchBackVector.z == 0) {
		}
		else {
			puchBackVector = puchBackVector * NormalizeMakeRotateMatrix(Inverse(worldTransform_.matWorld_));
			worldTransform_.translation_ += puchBackVector;
			worldTransform_.UpdateMatrix();
		}
	}

}

bool Box::IsCollision(Collider& otherCollider) {
	Vector3 puchBackVector;
	if (collider_->Collision(otherCollider, puchBackVector)) {
		return true;
	}
	return false;
}

bool Box::IsSetPerfect(Collider& otherCollider) {
	Vector3 puchBackVector;
	//当たったか否か
	if (collider_->Collision(otherCollider, puchBackVector)) {

		//誤差
		float ErrorNum = 0.001f;

		//誤差レベルの数字は0にする
		if (puchBackVector.x<ErrorNum && puchBackVector.x>-ErrorNum) {
			puchBackVector.x = 0;
		}
		if (puchBackVector.y<ErrorNum && puchBackVector.y>-ErrorNum) {
			puchBackVector.y = 0;
		}
		if (puchBackVector.z<ErrorNum && puchBackVector.z>-ErrorNum) {
			puchBackVector.z = 0;
		}

		//0
		if (puchBackVector.x == 0 && puchBackVector.y == 0 && puchBackVector.z == 0) {
			return true;
		}
		else {
			return false;
		}

		
	}
	return false;
}

bool Box::CollisionUnderCollider(Collider& other) {


	Vector3 offset = { 0.0f,-2.0f,0.0f };

	offset = offset * NormalizeMakeRotateMatrix(Inverse(worldTransform_.matWorld_));

	underCollider_->worldTransform_.translation_ = offset;

	underCollider_->AdjustmentScale();

	Vector3 puchBackVector;
	if (underCollider_->Collision(other,puchBackVector)) {

		//誤差
		float ErrorNum = 0.001f;

		//誤差レベルの数字は0にする
		if (puchBackVector.x<ErrorNum && puchBackVector.x>-ErrorNum) {
			puchBackVector.x = 0;
		}
		if (puchBackVector.y<ErrorNum && puchBackVector.y>-ErrorNum) {
			puchBackVector.y = 0;
		}
		if (puchBackVector.z<ErrorNum && puchBackVector.z>-ErrorNum) {
			puchBackVector.z = 0;
		}

		//ぴったり隣にある
		if (puchBackVector.x == 0 && puchBackVector.y == 0 && puchBackVector.z == 0) {
			return false;
		}
		else {
			//止まってる状態
			isBuried_ = true;
			return true;
		}

		
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

void Box::StateChange() {

	if (isBuried_) {
		state_ = kStay;
	}
	else {
		state_ = kFall;
	}
}
