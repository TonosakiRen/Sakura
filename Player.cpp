#include "Player.h"
#include "ImGuiManager.h"
#include "Map.h"
#include "Easing.h"
#include "Audio.h"

void Player::Initialize(const std::string name, ViewProjection* viewProjection, DirectionalLight* directionalLight,const WorldTransform& pWorld) {
	//初期化
	GameObject::Initialize(name, viewProjection, directionalLight);
	audio_ = Audio::GetInstance();
	input_ = Input::GetInstance();
	collider_.Initialize(&worldTransform_, name, viewProjection, directionalLight);
	underCollider_.Initialize(&worldTransform_, name, viewProjection, directionalLight);

	upCollider_.Initialize(&worldTransform_, name, viewProjection, directionalLight);
	animationTransform_.Initialize();
	animationTransform_.SetParent(&worldTransform_);
	animationTransform_.SetIsScaleParent(false);


	for (int i = 0; i < slimeNum; i++) {
		slimeTransform[i].Initialize();
		slimeTransform[i].SetIsScaleParent(false);
		slimeTransform[i].SetParent(&animationTransform_);
		slimeTransform[i].translation_.y = 1.5f - (3.0f / Player::slimeNum) * i - ((3.0f / Player::slimeNum) / 2.0f);
		slimeTransform[i].UpdateMatrix();
	}


	//座標系処理
	worldTransform_ = pWorld;
	worldTransform_.scale_ = portraitScale;

	//worldTransform_.translation_ = MakeTranslation(worldTransform_.matWorld_);
	//worldTransform_.SetParent(nullptr);

	//速度初期設定
	velocisity_ = { 0.0f,0.0f,0.0f };

	//加速度初期
	acceleration_ = { 0.0f,-0.04f,0.0f };
	//acceleration_ = { 0.0f,0.00f,0.0f };

	//ライティング無効化
	material_.enableLighting_ = false;

	slimeVerticalModel_.Initialize("Slime");
	slimeWideModel_.Initialize("SlimeWide");
}

void Player::StageInitialize(const WorldTransform& pWorld)
{
	velocisity_ = { 0.0f,0.0f,0.0f };
	acceleration_ = { 0.0f,-0.04f,0.0f };
	worldTransform_ = pWorld;
	worldTransform_.scale_ = portraitScale;
	worldTransform_.UpdateMatrix();
	rectangleState_ = RectangleFacing::kPortrait;
	isDead_ = false;
	isJump = false;
	rectangleState_ = RectangleFacing::kPortrait;
	rotateNum_ = 0;
	isGoal_ = false;
	isFinishGoalAnimation_ = false;
	goalT_ = 0.0f;
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
	if (!Map::isRotating && isGoal_ == false) {
		
#pragma region キーボード入力
		//入力による移動
		if (input_->PushKey(DIK_A) || input_->GetLStick().x < -10000) {
			if (isMoveSound_ == true) {
				isMoveSound_ = false;
				size_t handle = audio_->SoundLoadWave("purunn.wav");
				size_t purunnHandle = audio_->SoundPlayWave(handle);
				audio_->SetValume(purunnHandle, 0.1f);
			}

			move_.x -= spd_;
		}
		if (input_->PushKey(DIK_D) || input_->GetLStick().x > 10000) {
			if (isMoveSound_ == false) {
				isMoveSound_ = true;
				size_t handle = audio_->SoundLoadWave("purunn.wav");
				size_t purunnHandle = audio_->SoundPlayWave(handle);
				audio_->SetValume(purunnHandle, 0.1f);
			}
			move_.x += spd_;
		}

		//ジャンプ処理
		if (velocisity_.y == 0.0f ) {
			isJump = false;
		}

		if (velocisity_.y < 0.0f) {
			isJump = true;
		}

		if ((input_->TriggerKey(DIK_SPACE)|| input_->TriggerButton(XINPUT_GAMEPAD_A))&& isJump == false) {
			stateRequest_ = PlayerState::kJump;
			if (rectangleState_ == RectangleFacing::kPortrait) {
				size_t jumpHandle = audio_->SoundLoadWave("Jump_Smart.wav");
				size_t jumpPlayHandle = audio_->SoundPlayWave(jumpHandle);
				audio_->SetValume(jumpPlayHandle, 0.1f);
			}
			else {
				size_t jumpHandle = audio_->SoundLoadWave("Jump_Fat.wav");
				size_t jumpPlayHandle = audio_->SoundPlayWave(jumpHandle);
				audio_->SetValume(jumpPlayHandle, 0.1f);
			}
	
			isJump = true;
		}
#pragma endregion

		//状態更新
		UpdateState();

		
		//移動量に合わせて更新
		move_ += velocisity_;
		move_ = move_ * NormalizeMakeRotateMatrix(Inverse(worldTransform_.GetParent()->matWorld_));
		//加算
		worldTransform_.translation_ += move_;

	}

	UpdateMatiries();

	if (Map::rotateComplete) {
		if (rectangleState_ != RectangleFacing::kPortrait) {
			rectangleState_ = RectangleFacing::kPortrait;
		}
		else if (rectangleState_ != RectangleFacing::kLandscape) {
			rectangleState_ = RectangleFacing::kLandscape;
		}
	}

	Vector3 pos = MakeTranslation(worldTransform_.matWorld_);

	const float deadLine = 35.0f;
	//死亡判定
	if (pos.y <= -deadLine) {
		isDead_ = true;
	}
	if (isGoal_ == true) {

		worldTransform_.translation_ = Easing::easing(goalT_, worldTransform_.translation_, goalPos_, 0.03f);
		
		if (goalT_ >= 1.0f) {
			isFinishGoalAnimation_ = true;
		}
	}

}

void Player::UpdateMatiries() {

	//行列更新
	worldTransform_.UpdateMatrix();

	//コライダー更新
	collider_.AdjustmentScale();
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
		float ErrorNum = 0.00001f;

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
		float ErrorNum = 0.01f;

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

		//ぴったりはゆるしまへん
		if (puchBackVector.x == 0 && puchBackVector.y == 0 && puchBackVector.z == 0) {
			return false;
		}
		else {
			//ぴったりじゃないのでOK
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

	uint32_t index = rotateNum_ % 4;
	Matrix4x4 RotateZMatrix = MakeIdentity4x4();
	RotateZMatrix = MakeRotateZMatrix(Radian(90.0f) * index);

	bool isModelDraw = false;

	//変形アニメーションここに書いちゃお
	if (Map::isRotating && !isChangeScaled) {
		isChangeScaled = true;
		if (rectangleState_ == RectangleFacing::kPortrait) {
			animationTransform_.scale_ = portraitScale * RotateZMatrix;
		}
		else {
			animationTransform_.scale_ = landScapeScale * RotateZMatrix;
		}
	}

	if (Map::isRotating) {
		isModelDraw = true;
	}
	else {
		isChangeScaled = false;
	}
	
	if (isChangeAnimationRect_ == true) {
		animationTransform_.scale_ = Easing::easing(changeRectT, portraitScale, landScapeScale, 0.03f, Easing::easeOutElastic) * RotateZMatrix;
		if (changeRectT >= 1.0f) {
			isChangeAnimationRect_ = false;
		}
		isModelDraw = true;
	}
	else {
		changeRectT = 0.0f;
	}

	if (!isModelDraw) {
		animationTransform_.scale_ = { 1.0f,1.0f,1.0f };
	}

	animationTransform_.UpdateMatrix();
	if (!isModelDraw) {
		for (int i = 0; i < slimeNum; i++) {
			slimeTransform[i].UpdateMatrix();
			if (rectangleState_ == RectangleFacing::kPortrait) {
				slimeVerticalModel_.Draw(slimeTransform[i], *viewProjection_, *directionalLight_, material_);
			}
			else {
				slimeWideModel_.Draw(slimeTransform[i], *viewProjection_, *directionalLight_, material_);
			}
		}
	}
	else {
		model_.Draw(animationTransform_, *viewProjection_, *directionalLight_, material_);
	}

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
			if (rectangleState_ == RectangleFacing::kPortrait) {
				offset = { 0.0f,0.65f,0.0f };
				velocisity_ = offset;
			}

			if (rectangleState_ == RectangleFacing::kLandscape) {
				offset = { 0.0f,0.5f,0.0f };
				velocisity_ = offset;
			}
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
		velocisity_.y = clamp(velocisity_.y, -0.3f, 200.0f);
		velocisity_ += acceleration_;
		break;
	default:
		break;
	}
}

