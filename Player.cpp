#include "Player.h"
#include "ImGuiManager.h"
#include "Map.h"

void Player::Initialize(const std::string name, ViewProjection* viewProjection, DirectionalLight* directionalLight, WorldTransform pWorld) {
	GameObject::Initialize(name, viewProjection, directionalLight);
	input_ = Input::GetInstance();
	collider_.Initialize(&worldTransform_, name, viewProjection, directionalLight);
	worldTransform_ = pWorld;
	velocisity_ = { 0.0f,0.0f,0.0f };
	acceleration_ = { 0.0f,-0.05f,0.0f };
	//acceleration_ = { 0.0f,0.00f,0.0f };
	material_.enableLighting_ = false;
}

void Player::Update() {
	ImGui::Begin("player");
	ImGui::DragFloat3("pos", &worldTransform_.translation_.x, 0.01f);
	ImGui::End();


	// マップが動いていないの処理
	if (!Map::isRotating) {
		Vector3 move = { 0.0f,0.0f,0.0f };

		if (input_->PushKey(DIK_A)) {
			move.x -= 0.3f;
		}
		if (input_->PushKey(DIK_D)) {
			move.x += 0.3f;
		}
		if (input_->PushKey(DIK_W)) {
			move.y += 0.3f;
		}
		if (input_->PushKey(DIK_S)) {
			move.y -= 0.3f;
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
}

void Player::Collision(Collider& otherCollider) {
	Vector3 puchBackVector;
	if (collider_.Collision(otherCollider, puchBackVector)) {
		worldTransform_.translation_ += puchBackVector;
		worldTransform_.UpdateMatrix();
	}

	//仮でフラグ解除
	stateRequest_ = State::kNormal;
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
