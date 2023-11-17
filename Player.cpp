#include "Player.h"
#include "ImGuiManager.h"
#include "Map.h"

void Player::Initialize(const std::string name, ViewProjection* viewProjection, DirectionalLight* directionalLight, WorldTransform pWorld) {
	//初期化
	GameObject::Initialize(name, viewProjection, directionalLight);
	input_ = Input::GetInstance();
	collider_.Initialize(&worldTransform_, name, viewProjection, directionalLight);
	underCollider_.Initialize(&worldTransform_, name, viewProjection, directionalLight);

	//座標系処理
	worldTransform_ = pWorld;
	//worldTransform_.translation_ = MakeTranslation(worldTransform_.matWorld_);
	//worldTransform_.SetParent(nullptr);

	//速度初期設定
	velocisity_ = { 0.0f,0.0f,0.0f };

	//加速度初期
	acceleration_ = { 0.0f,-0.05f,0.0f };
	//acceleration_ = { 0.0f,0.00f,0.0f };

	//ライティング無効化
	material_.enableLighting_ = false;
}

void Player::Update() {

#ifdef _DEBUG
	//デバッグ描画
	ImGui::Begin("player");
	ImGui::DragFloat3("pos", &worldTransform_.translation_.x, 0.01f);
	ImGui::Text("state : %d", state_);
	ImGui::End();
#endif // _DEBUG

	//コリジョン処理を行ったかのフラグを初期化
	isBuried_ = false;

	// マップが動いていない時の処理
	if (!Map::isRotating) {
		Vector3 move = { 0.0f,0.0f,0.0f };

#pragma region キーボード入力
		//入力による移動
		if (input_->PushKey(DIK_A)) {
			move.x -= spd_;
		}
		if (input_->PushKey(DIK_D)) {
			move.x += spd_;
		}
		if (input_->PushKey(DIK_W)) {
			move.y += spd_;
		}
		if (input_->PushKey(DIK_S)) {
			move.y -= spd_;
		}

		//ジャンプ処理
		if (input_->TriggerKey(DIK_SPACE)) {
			stateRequest_ = PlayerState::kJump;
		}
#pragma endregion

		//状態更新
		UpdateState();

		//移動量に合わせて更新
		move += velocisity_;
		move = move * NormalizeMakeRotateMatrix(Inverse(worldTransform_.matWorld_));
		//加算
		worldTransform_.translation_ += move;

	}

	//コライダー更新
	collider_.AdjustmentScale();

	//下の位置に配置して更新
	Vector3 offset = { 0.0f,-2.0f,0.0f };
	offset = offset * NormalizeMakeRotateMatrix(Inverse(worldTransform_.matWorld_));
	underCollider_.worldTransform_.translation_ = offset;
	underCollider_.AdjustmentScale();

	//行列更新
	worldTransform_.UpdateMatrix();

	//過去情報を渡す
	preIsRotating_ = Map::isRotating;
}

void Player::Collision(Collider& otherCollider) {
	Vector3 puchBackVector;
	if (collider_.Collision(otherCollider, puchBackVector)) {

		if (puchBackVector.x == 0 && puchBackVector.y == 0 && puchBackVector.z == 0) {

		}
		else {
			puchBackVector = puchBackVector * NormalizeMakeRotateMatrix(Inverse(worldTransform_.matWorld_));
			worldTransform_.translation_ += puchBackVector;
			worldTransform_.UpdateMatrix();
			//処理フラグをON
			isBuried_ = true;
		}
	}
}

bool Player::IsSetPerfect(Collider& otherCollider) {
	Vector3 puchBackVector;
	//当たったか否か
	if (collider_.Collision(otherCollider, puchBackVector)) {

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

		return false;
	}
	return false;
}

bool Player::IsCollision(Collider& otherCollider) {
	Vector3 puchBackVector;
	if (collider_.Collision(otherCollider, puchBackVector)) {
		//ぴったり隣の場合
		if (puchBackVector.x == 0 && puchBackVector.y == 0 && puchBackVector.z == 0) {
			return false;
		}
		//違う場合
		return true;
	}
	return false;
}

void Player::UnderColliderCollision(Collider& otherCollider) {

	//コリジョンした場合の更新位置をチェック
	Vector3 offset = { 0.0f,-2.0f,0.0f };
	offset = offset * NormalizeMakeRotateMatrix(Inverse(worldTransform_.matWorld_));
	underCollider_.worldTransform_.translation_ = offset;
	underCollider_.AdjustmentScale();

	Vector3 puchBackVector;
	if (underCollider_.Collision(otherCollider, puchBackVector)) {

		if (puchBackVector.x == 0 && puchBackVector.y == 0 && puchBackVector.z == 0) {

		}
		else {
			puchBackVector = puchBackVector * NormalizeMakeRotateMatrix(Inverse(worldTransform_.matWorld_));
			worldTransform_.translation_ += puchBackVector;
			worldTransform_.UpdateMatrix();
			//仮でフラグ解除
			stateRequest_ = PlayerState::kNormal;
		}
	}
}

bool Player::IsUnderColliderCollision(Collider& otherCollider) {

	Vector3 puchBackVector;
	if (underCollider_.Collision(otherCollider, puchBackVector)) {

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


		if (puchBackVector.x == 0 && puchBackVector.y == 0 && puchBackVector.z == 0) {
			return false;
		}
		else {
			return true;
		}
	}
	return false;
}

void Player::Draw() {
	collider_.Draw();
	underCollider_.Draw();

	GameObject::Draw();

}

void Player::UnderColliderUpdate() {
	//コライダーを配置
	Vector3 offset = { 0.0f,-2.0f,0.0f };
	offset = offset * NormalizeMakeRotateMatrix(Inverse(worldTransform_.matWorld_));
	underCollider_.worldTransform_.translation_ = offset;
	underCollider_.AdjustmentScale();
}

/*
void Player::StateChange(const std::vector<std::unique_ptr<>>& boxes) {
	//押し出し処理が行われていた場合
	//if (isBuried_) {

	//押し出されたプレイヤーに合わせてコライダーを配置
	Vector3 offset = { 0.0f,-2.0f,0.0f };
	offset = offset * NormalizeMakeRotateMatrix(Inverse(worldTransform_.matWorld_));
	underCollider_.worldTransform_.translation_ = offset;
	underCollider_.AdjustmentScale();

	//Boxと床のコリジョン処理
	for (auto& box : boxes) {
		//アンダーコライダーと当たっているか否か
		if (IsUnderColliderCollision(*box->GetCollider())) {
			//当たっている場合
			//押し出し量０で当たっていたら
 			if (IsSetPerfect(*box->GetCollider())) {
				stateRequest_ = PlayerState::kNormal;
			}
			else {
				//当たっていない場合落下
				if (state_ != PlayerState::kJump) {
					//初期化処理を行わず状態を変更変更
					state_ = PlayerState::kJump;
					//初期速度を設定
					velocisity_ = { 0.0f,0.0f,0.0f };
				}
			}

		}
		else {
			//当たっていない場合落下
			if (state_ != PlayerState::kJump) {
				//初期化処理を行わず状態を変更変更
				state_ = PlayerState::kJump;
				//初期速度を設定
				velocisity_ = { 0.0f,0.0f,0.0f };
			}
		}
	}

	//}
}
*/

void Player::UpdateState() {
	//状態変更時の初期化処理
	if (stateRequest_) {
		state_ = stateRequest_.value();

		switch (state_) {
		case PlayerState::kNormal:
			velocisity_ = { 0.0f,0.0f,0.0f };
			break;
		case PlayerState::kJump:
			velocisity_.y = 1.0f;
			break;
		default:
			break;
		}
		stateRequest_ = std::nullopt;
	}

	//状態更新処理
	switch (state_) {
	case PlayerState::kNormal:
		break;
	case PlayerState::kJump:
		velocisity_.y = clamp(velocisity_.y, -0.5f, 200.0f);
		velocisity_ += acceleration_;
		break;
	default:
		break;
	}
}

