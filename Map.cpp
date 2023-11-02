#include "Map.h"
#include "Mymath.h"
#include <cassert>
#include "ImGuiManager.h"
#include <stdio.h>

bool Map::isRotating = false;

void Map::Initialize(const std::string name, ViewProjection* viewProjection, DirectionalLight* directionalLight) {

	input = Input::GetInstance();
	
	GameObject::Initialize(name, viewProjection, directionalLight);
	SetEnableLighting(false);

	// 回転中心点の検索
	Vector3 centerPos = { mapTileNumX_ - tileWide_ / 2, -mapTileNumY_ + tileWide_ / 2, 0 };

	worldTransform_.Initialize();
	worldTransform_.translation_ = centerPos;
	worldTransform_.UpdateMatrix();

	// マップタイルによる座標設定
	for (int tileY = 0; tileY < mapTileNumY_; tileY++) {
		for (int tileX = 0; tileX < mapTileNumX_; tileX++) {
			if (mapTile_[tileY][tileX] == Block) {
				WorldTransform world;
				world.Initialize();
				world.translation_ = { tileWide_ * tileX, -tileWide_ * tileY, 0 };

				// 親子関係設定
				world.SetParent(&worldTransform_);
				
				// ベクトル差分を代入
				Vector3 subPos = Subtract(world.translation_, worldTransform_.translation_);

				// 座標代入
				world.translation_ = subPos;

				world.UpdateMatrix();

				//Collider
				Collider collider;
				collider.Initialize(&world,"blockTile", viewProjection_, directionalLight_);

				
				worlds_.push_back(world);
				colliders_.push_back(collider);
			}

			if (mapTile_[tileY][tileX] == Player) {

				playerWorld_.Initialize();

				playerWorld_.translation_ = { tileWide_ * tileX, -tileWide_ * tileY, 0 };
				playerWorld_.SetParent(&worldTransform_);

				playerWorld_.UpdateMatrix();
				// ベクトル差分を代入
				Vector3 subPos = Subtract(playerWorld_.translation_, worldTransform_.translation_);

				// 座標代入
				playerWorld_.translation_ = subPos;

				playerWorld_.UpdateMatrix();
			}
		}
	}
}

void Map::Update() {
	ImGui::Begin("map");
	ImGui::DragFloat3("map world pos", &worldTransform_.translation_.x, 0.1f);
	ImGui::DragFloat3("rotate", &worldTransform_.rotation_.x, 0.1f);
	ImGui::DragFloat3("map world scale", &worldTransform_.scale_.x, 0.1f);
	ImGui::End();

	// 状態リクエスト処理
	RequestProcessing();

	StateUpdate();

	worldTransform_.UpdateMatrix();
	for (WorldTransform world : worlds_) {
		world.UpdateMatrix();
	}
}

void Map::Draw() {

	// 壁描画
	for (WorldTransform world : worlds_) {
		GameObject::Draw(world);
	}

	// 描画
	GameObject::Draw(worldTransform_);
}

void Map::RequestProcessing() {
	// 状態変更リクエストがある場合
	if (stateRequest_) {
		state_ = stateRequest_.value();
		switch (state_) {
		case Map::State::kNormal:
			InitializeStateNormal();
			break;
		case Map::State::kRightRotation:
			InitializeStateRightRotation();
			break;
		case Map::State::kLeftRotation:
			InitializeStateLeftRotation();
			break;
		default:
			break;
		}
		// リクエストリセット
		stateRequest_ = std::nullopt;
	}
}

void Map::StateUpdate() {

	switch (state_) {
	case Map::State::kNormal:
		UpdateStateNormal();
		break;
	case Map::State::kRightRotation:
		UpdateStateRightRotation();
		break;
	case Map::State::kLeftRotation:
		UpdateStateLeftRotation();
		break;
	default:
		break;
	}
}

void Map::InitializeStateNormal() { isRotating = false; }

void Map::InitializeStateRightRotation() {
	isRotating = true;

	// 開始回転と終了回転設定
	rotateE_ = { .startT{worldTransform_.rotation_.z},
		.endT{worldTransform_.rotation_.z + HalfPI_} };

	t_ = 0;
}

void Map::InitializeStateLeftRotation() {
	isRotating = true;

	// 開始回転と終了回転設定
	rotateE_ = { .startT{worldTransform_.rotation_.z}, .endT{worldTransform_.rotation_.z - HalfPI_} };

	t_ = 0;
}

void Map::UpdateStateNormal() {

	if (input->PushKey(DIK_E)) {
		stateRequest_ = State::kRightRotation;
	}
	if (input->PushKey(DIK_Q)) {
		stateRequest_ = State::kLeftRotation;
	}
}

float EsingFloat(const float t, const float start, const float end) {
	return start * (1.0f - t) + end * t;
}

void Map::UpdateStateRightRotation() {

	// 回転量変更
	worldTransform_.rotation_.z = EsingFloat(t_, rotateE_.startT, rotateE_.endT);
	// T加算
	t_ += addT_;
	// １以上で終了
	if (t_ >= 1.0f) {
		// 終点に整える
		t_ = 1.0f;
		worldTransform_.rotation_.z = EsingFloat(t_, rotateE_.startT, rotateE_.endT);
		// 状態変更
		stateRequest_ = State::kNormal;
	}
}

void Map::UpdateStateLeftRotation() {

	// 回転量変更
	worldTransform_.rotation_.z = EsingFloat(t_, rotateE_.startT, rotateE_.endT);
	// T加算
	t_ += addT_;
	// １以上で終了
	if (t_ >= 1.0f) {
		// 終点に整える
		t_ = 1.0f;
		worldTransform_.rotation_.z = EsingFloat(t_, rotateE_.startT, rotateE_.endT);
		// 状態変更
		stateRequest_ = State::kNormal;
	}
}
