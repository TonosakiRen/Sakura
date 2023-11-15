#include "Player.h"
#include "ImGuiManager.h"
#include "Map.h"

void Player::Initialize(const std::string name, ViewProjection* viewProjection, DirectionalLight* directionalLight, WorldTransform pWorld) {
	GameObject::Initialize(name, viewProjection, directionalLight);
	input_ = Input::GetInstance();
	collider_.Initialize(&worldTransform_, name, viewProjection, directionalLight);
	
	//座標系処理
	worldTransform_ = pWorld;
	//worldTransform_.translation_ = MakeTranslation(worldTransform_.matWorld_);
	//worldTransform_.SetParent(nullptr);

	//マップ中心点を取得
	mapCenterTranslation_ = pWorld.GetParent();

	velocisity_ = { 0.0f,0.0f,0.0f };
	acceleration_ = { 0.0f,-0.05f,0.0f };
	//acceleration_ = { 0.0f,0.00f,0.0f };
	material_.enableLighting_ = false;
}

void Player::Update() {
	ImGui::Begin("player");
	ImGui::DragFloat3("pos", &worldTransform_.translation_.x, 0.01f);
	ImGui::End();

	//UpdateMapRotating();

	// マップが動いていないの処理
	if (!Map::isRotating) {
		Vector3 move = { 0.0f,0.0f,0.0f };

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

		if (input_->TriggerKey(DIK_SPACE)) {
			stateRequest_ = State::kJump;
		}
		
		UpdateState();

		collider_.AdjustmentScale();
		
		move += velocisity_;

		move = move * NormalizeMakeRotateMatrix(Inverse(worldTransform_.matWorld_));

		worldTransform_.translation_ += move;

	}

	worldTransform_.UpdateMatrix();

	preIsRotating_ = Map::isRotating;
}

void Player::Collision(Collider& otherCollider) {
	Vector3 puchBackVector;
	if (collider_.Collision(otherCollider, puchBackVector)) {
		puchBackVector = puchBackVector * NormalizeMakeRotateMatrix(Inverse(worldTransform_.matWorld_));
		worldTransform_.translation_ += puchBackVector;
		worldTransform_.UpdateMatrix();
		//仮でフラグ解除
		stateRequest_ = State::kNormal;
	}
}

bool Player::IsCollision(Collider& otherCollider) {
	Vector3 puchBackVector;
	if (collider_.Collision(otherCollider, puchBackVector)) {
		return true;
	}
	return false;
}

void Player::Draw() {
	collider_.Draw();
	GameObject::Draw();
}

void Player::UpdateState() {

	if (stateRequest_) {
		state_ = stateRequest_.value();
		switch (state_) {
		case Player::State::kNormal:
			velocisity_ = { 0.0f,0.0f,0.0f };
			break;
		case Player::State::kJump:
			velocisity_.y = 1.0f;
			break;
		default:
			break;
		}



		stateRequest_ = std::nullopt;
	}



	switch (state_) {
	case Player::State::kNormal:
		break;
	case Player::State::kJump:
		velocisity_.y = clamp(velocisity_.y, -0.5f, 200.0f);
		velocisity_ += acceleration_;
		break;
	default:
		break;
	}
}

void Player::UpdateMapRotating() {
	//回転し始めた瞬間の処理
	if (!preIsRotating_ && Map::isRotating) {
		//マップから見た位置を取得
		Vector3 subPos = MakeTranslation(worldTransform_.matWorld_) -MakeTranslation(mapCenterTranslation_->matWorld_);
		//代入
		worldTransform_.translation_ = subPos;
		//親子関係追加
		worldTransform_.SetParent(mapCenterTranslation_);

		//更新
		worldTransform_.UpdateMatrix();
	}
	
	//回転処理が終わった時
	if (preIsRotating_ && !Map::isRotating) {

		//現在のワールド座標をローカル座標に代入
		worldTransform_.translation_ = MakeTranslation(worldTransform_.matWorld_);
		//親子関係削除
		worldTransform_.SetParent(nullptr);
		//更新
		worldTransform_.UpdateMatrix();
	}
}
