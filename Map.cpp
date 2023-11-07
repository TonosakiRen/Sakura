#include "Map.h"
#include "Mymath.h"
#include <cassert>
#include "ImGuiManager.h"
#include <stdio.h>

#include <stdio.h>
#include<ctype.h>
#include<list>
#include<cassert>

std::list<std::list<int>> LoadMapData(const char* fileName) {
	//返すデータの作成
	std::list<std::list<int>>mapData;

	//ファイルの読み込み
	FILE* fp;
	int err = fopen_s(&fp, fileName, "r");


	//読み込めなかったら返す
	if (err != 0) {

		assert("%s は見つかりませんでした!zako!");
	}
	else {

		//取得した文字いれる変数
		char c;
		//仮使用
		std::list<int> data;
		//	EOFまでファイルから文字を1文字ずつ読み込む
		while ((c = fgetc(fp)) != EOF) {

			//次の行へ
			if (c == '}') {
				//今までの行の数字のリストを代入
				mapData.push_back(data);
				//全削除
				data.clear();

				continue;
			}

			//数字か否か
			if (isdigit(c)) {

				//各数字に直す
				int num = 0;
				if (c == '0') {
					num = 0;
				}
				else if (c == '1') {
					num = 1;
				}
				else if (c == '2') {
					num = 2;
				}
				else if (c == '3') {
					num = 3;
				}
				else if (c == '4') {
					num = 4;
				}
				else if (c == '5') {
					num = 5;
				}
				else if (c == '6') {
					num = 6;
				}
				else if (c == '7') {
					num = 7;
				}
				else if (c == '8') {
					num = 8;
				}
				else if (c == '9') {
					num = 9;
				}

				//データ型に挿入
				data.push_back(num);
			}
		}
	}

	//ファイルを閉じる
	fclose(fp);


	return mapData;
}


bool Map::isRotating = false;

void Map::Initialize(const std::string name, ViewProjection* viewProjection, DirectionalLight* directionalLight) {

	input_ = Input::GetInstance();
	
	GameObject::Initialize(name, viewProjection, directionalLight);
	SetEnableLighting(false);

	//マップデータを読み込み
	mapData_ = LoadMapData(map1Pass);
	//xに一番多いサイズ取得
	for (auto& mapdataX : mapData_) {
		if (mapTileNumX_ < (int)mapdataX.size()) {
			mapTileNumX_ = (int)mapdataX.size();
		}
	}
	mapTileNumY_ = (int)mapData_.size();


	// 回転中心点の検索
	Vector3 centerPos = { mapTileNumX_ - tileWide_ / 2, -mapTileNumY_ + tileWide_ / 2, 0 };

	worldTransform_.Initialize();
	worldTransform_.translation_ = centerPos;
	worldTransform_.UpdateMatrix();

	//Collider初期化のための変数
	uint32_t colliderAcceces = 0;

	
	int containerNumberY = 0;
	for (auto& mapDataY : mapData_) {
		int containerNumberX = 0;
		for (auto& mapDataX : mapDataY) {
			if (mapDataX == Block) {
				std::unique_ptr<WorldTransform> world;
				world = std::make_unique<WorldTransform>();
				world->Initialize();
				world->translation_ = { tileWide_ * containerNumberX, -tileWide_ * containerNumberY, 0 };

				// 親子関係設定
				world->SetParent(&worldTransform_);

				// ベクトル差分を代入
				Vector3 subPos = Subtract(world->translation_, worldTransform_.translation_);

				// 座標代入
				world->translation_ = subPos;

				world->UpdateMatrix();

				//Collider
				std::unique_ptr<Collider> collider;
				collider = std::make_unique<Collider>();

				WallWorlds_.emplace_back(std::move(world));
				collider->Initialize(WallWorlds_[colliderAcceces].get(), "blockTile", viewProjection_, directionalLight_);
				colliders_.push_back(std::move(collider));
				colliderAcceces++;
			}

			if (mapDataX == Player) {

				playerWorld_.Initialize();

				playerWorld_.translation_ = { tileWide_ * containerNumberX, -tileWide_ * containerNumberY, 0 };
				playerWorld_.SetParent(&worldTransform_);

				playerWorld_.UpdateMatrix();
				// ベクトル差分を代入
				Vector3 subPos = Subtract(playerWorld_.translation_, worldTransform_.translation_);

				// 座標代入
				playerWorld_.translation_ = subPos;

				playerWorld_.UpdateMatrix();
			}
			
			containerNumberX++;
		}
		containerNumberY++;
	}

	/*
	// マップタイルによる座標設定
	for (int tileY = 0; tileY < mapTileNumY_; tileY++) {
		for (int tileX = 0; tileX < mapTileNumX_; tileX++) {
			if (mapTile_[tileY][tileX] == Block) {
				std::unique_ptr<WorldTransform> world;
				world = std::make_unique<WorldTransform>();
				world->Initialize();
				world->translation_ = { tileWide_ * tileX, -tileWide_ * tileY, 0 };

				// 親子関係設定
				world->SetParent(&worldTransform_);
				
				// ベクトル差分を代入
				Vector3 subPos = Subtract(world->translation_, worldTransform_.translation_);

				// 座標代入
				world->translation_ = subPos;

				world->UpdateMatrix();

				//Collider
				std::unique_ptr<Collider> collider;
				collider = std::make_unique<Collider>();
				
				WallWorlds_.emplace_back(std::move(world));
				collider->Initialize(WallWorlds_[colliderAcceces].get(), "blockTile", viewProjection_, directionalLight_);
				colliders_.push_back(std::move(collider));
				colliderAcceces++;
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
	*/
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
	for (auto& world : WallWorlds_) {
		world->UpdateMatrix();
	}
	for (auto& collider : colliders_) {
		collider->AdjustmentScale();
	}
}

void Map::Draw() {

	// 壁描画
	for (auto& world : WallWorlds_) {
		GameObject::Draw(*world);
	}

	// 描画
	GameObject::Draw(worldTransform_);

	for (auto& collider : colliders_) {
		collider->Draw();
	}
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

	if (input_->PushKey(DIK_E)) {
		stateRequest_ = State::kRightRotation;
	}
	if (input_->PushKey(DIK_Q)) {
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


