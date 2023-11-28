#include "Player.h"
#include "ImGuiManager.h"
#include "Map.h"

void Player::Initialize(const std::string name, ViewProjection* viewProjection, DirectionalLight* directionalLight, WorldTransform pWorld) {
	//初期化
	GameObject::Initialize(name, viewProjection, directionalLight);
	input_ = Input::GetInstance();
	collider_.Initialize(&worldTransform_, name, viewProjection, directionalLight);
	underCollider_.Initialize(&worldTransform_, name, viewProjection, directionalLight);

	upCollider_.Initialize(&worldTransform_, name, viewProjection, directionalLight);
	animationTransform_.Initialize();
	animationTransform_.SetParent(&worldTransform_);

	//座標系処理
	worldTransform_ = pWorld;
	worldTransform_.scale_ = portraitScale;

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
	ImGui::DragFloat3("rotate", &worldTransform_.rotation_.x, 0.01f);

	ImGui::Text("state : %d", state_);
	ImGui::DragFloat3("under C", &underCollider_.worldTransform_.translation_.x);

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

	//コリジョン処理を行ったかのフラグを初期化
	isBuried_ = false;


	move_ = { 0.0f,0.0f,0.0f };
	// マップが動いていない時の処理
	if (!Map::isRotating) {
		
#pragma region キーボード入力
		//入力による移動
		if (input_->PushKey(DIK_A)) {
			move_.x -= spd_;
		}
		if (input_->PushKey(DIK_D)) {
			move_.x += spd_;
		}
		if (input_->PushKey(DIK_W)) {
			move_.y += spd_;
		}
		if (input_->PushKey(DIK_S)) {
			move_.y -= spd_;
		}

		//ジャンプ処理
		if (input_->TriggerKey(DIK_SPACE)) {
			stateRequest_ = PlayerState::kJump;
		}
#pragma endregion

		//状態更新
		UpdateState();

		//移動量に合わせて更新
		move_ += velocisity_;
		move_ = move_ * NormalizeMakeRotateMatrix(Inverse(worldTransform_.matWorld_));
		//加算
		worldTransform_.translation_ += move_;

	}


	//行列更新
	worldTransform_.UpdateMatrix();

	//コライダー更新
	collider_.AdjustmentScale();	

	if (Map::rotateComplete) {
		if (rectangleState_ != RectangleFacing::kPortrait) {
			rectangleState_ = RectangleFacing::kPortrait;
		}
		else if (rectangleState_ != RectangleFacing::kLandscape) {
			rectangleState_ = RectangleFacing::kLandscape;
		}
	}
}

void Player::Collision(Collider& otherCollider, const Vector3& priotiyVector) {
	Vector3 puchBackVector;
	if (collider_.Collision(otherCollider, puchBackVector, priotiyVector)) {

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
	//下の位置に配置して更新
	Vector3 offset{};
	switch (rectangleState_) {
	case RectangleFacing::kPortrait:
		offset = { 0.0f,-2.5f,0.0f };
		offset = offset * NormalizeMakeRotateMatrix(Inverse(worldTransform_.matWorld_));

		underCollider_.worldTransform_.translation_ = offset;
		underCollider_.worldTransform_.scale_ = { 1.0f,1.0f,1.0f };
		underCollider_.AdjustmentScale();

		break;
	case RectangleFacing::kLandscape:
		offset = { 0.0f,-2.0f,0.0f };
		offset = offset * NormalizeMakeRotateMatrix(Inverse(worldTransform_.matWorld_));

		underCollider_.worldTransform_.translation_ =  offset;
		underCollider_.worldTransform_.scale_ = { 1.0f,1.0f,1.0f };
		underCollider_.AdjustmentScale();
		break;
	default:
		break;
	}

	
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

	//コリジョンした場合の更新位置をチェック
	//下の位置に配置して更新
	Vector3 offset{};
	switch (rectangleState_) {
	case RectangleFacing::kPortrait:
		offset = { 0.0f,-2.0f,0.0f };
		offset = offset * NormalizeMakeRotateMatrix(Inverse(worldTransform_.matWorld_));

		underCollider_.worldTransform_.translation_ = offset;
		underCollider_.worldTransform_.scale_ = { 1.0f,1.0f,1.0f };
		underCollider_.AdjustmentScale();

		break;
	case RectangleFacing::kLandscape:
		offset = { 0.0f,-2.0f,0.0f };
		offset = offset * NormalizeMakeRotateMatrix(Inverse(worldTransform_.matWorld_));

		underCollider_.worldTransform_.translation_ = offset;
		underCollider_.worldTransform_.scale_ = { 1.0f,1.0f,1.0f };
		underCollider_.AdjustmentScale();
		break;
	default:
		break;
	}


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

bool Player::IsUpColliderCollision(Collider& otherCollider) {
	//コライダーを配置
	Vector3 offset = { 0.0f,2.0f,0.0f };
	offset = offset * NormalizeMakeRotateMatrix(Inverse(worldTransform_.matWorld_));
	upCollider_.worldTransform_.translation_ =  offset;
	upCollider_.AdjustmentScale();


	Vector3 puchBackVector;
	if (upCollider_.Collision(otherCollider, puchBackVector)) {

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
	upCollider_.Draw();
	animationTransform_.UpdateMatrix();
	model_.Draw(animationTransform_, *viewProjection_, *directionalLight_,material_);
}




void Player::UpdateState() {
	//状態変更時の初期化処理
	if (stateRequest_) {
		state_ = stateRequest_.value();

		Vector3 offset;
		switch (state_) {
		case PlayerState::kNormal:
			velocisity_ = { 0.0f,0.0f,0.0f };
			break;
		case PlayerState::kJump:
			offset = { 0.0f,1.0f,0.0f };
			//offset = offset * NormalizeMakeRotateMatrix(Inverse(worldTransform_.matWorld_));
			velocisity_ = offset;
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

