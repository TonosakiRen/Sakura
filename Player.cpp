#include "Player.h"
#include "ImGuiManager.h"
#include "Map.h"

void Player::Initialize(const std::string name, ViewProjection* viewProjection, DirectionalLight* directionalLight, WorldTransform pWorld)
{
	GameObject::Initialize(name,viewProjection, directionalLight);
	input_ = Input::GetInstance();
	collider_.Initialize(&worldTransform_,name, viewProjection, directionalLight);
	worldTransform_ = pWorld;
	velocisity_ = { 0.0f,0.0f,0.0f };
	acceleration_ = { 0.0f,-0.05f,0.0f };
	acceleration_ = { 0.0f,0.00f,0.0f };
	material_.enableLighting_ = false;
}

void Player::Update()
{
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
			velocisity_.y = 1.0f;
		}

		move = move * NormalizeMakeRotateMatrix(Inverse(worldTransform_.matWorld_));

		worldTransform_.translation_ += move;

		collider_.AdjustmentScale();
		velocisity_.y = clamp(velocisity_.y, -0.5f, 200.0f);
		velocisity_ += acceleration_;
		worldTransform_.translation_ += velocisity_;
	}

	worldTransform_.UpdateMatrix();
}

void Player::Collision(Collider& otherCollider)
{
	Vector3 puchBackVector;
	if (collider_.Collision(otherCollider, puchBackVector)) {
		worldTransform_.translation_ += puchBackVector;
		worldTransform_.UpdateMatrix();
	}

}

void Player::Draw() {
	collider_.Draw();
	GameObject::Draw();
}