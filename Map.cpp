#include "Map.h"
#include "Mymath.h"
#include <cassert>
#include "ImGuiManager.h"
#include <stdio.h>

#include <stdio.h>
#include<ctype.h>
#include<list>
#include<cassert>

std::vector<std::vector<int>> LoadMapData(const char* fileName) {
	//返すデータの作成
	std::vector<std::vector<int>>mapData;

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
		std::vector<int> data;
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

bool Map::preIsRotating = false;

bool Map::rotateComplete = false;

Map::Map()
{
	particleBox_ = std::make_unique<ParticleBox>(kBoxNum);
}

void Map::Initialize(const std::string name, ViewProjection* viewProjection, DirectionalLight* directionalLight,int num) {

	input_ = Input::GetInstance();

	GameObject::Initialize(name, viewProjection, directionalLight);
	SetEnableLighting(false);
	particleBox_->Initialize();
	mapPassNumber_ = num;

	//マップデータを読み込み
	mapData_ = LoadMapData(map1Pass[num]);

	MapPositioningInitialize();

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

void Map::StageInitialize(int num)
{
	mapData_ = LoadMapData(map1Pass[num]);
	MapPositioningInitialize();
	state_ = State::kNormal;
	stateRequest_ = std::nullopt;
	t_ = 0;
}

void Map::Update() {
	ImGuiDraw();

	if (preIsRotating == isRotating) {
		rotateComplete = false;
	}
	//編集モードがoffの時処理
	if (!isEditOn_) {
		// 状態リクエスト処理
		RequestProcessing();

		StateUpdate();
	}
	else {
		//GameSceneにある
		//MapEditor();
	}


	//行列更新
	worldTransform_.UpdateMatrix();
	for (auto& world : WallWorlds_) {
		world->UpdateMatrix();
	}
	for (auto& collider : colliders_) {
		collider->AdjustmentScale();
	}


	preIsRotating = isRotating;
}

void Map::Draw() {

	std::vector<ParticleBox::InstancingBufferData> instancingBufferDatas;
	instancingBufferDatas.reserve(kBoxNum);

	int worldLocation = 0;
	for (int32_t y = 0; y < mapData_.size(); y++) {
		for (int32_t x = 0; x < mapData_[y].size(); x++) {
			if (mapData_[y][x] == Block) {
				//壁の描画
				/*GameObject::Draw(*WallWorlds_[worldLocation]);*/
				instancingBufferDatas.emplace_back(WallWorlds_[worldLocation]->matWorld_);
			}
			worldLocation++;
		}
	}

	if (!instancingBufferDatas.empty()) {
		particleBox_->Draw(instancingBufferDatas, *viewProjection_, *directionalLight_,{1.0f,1.0f,1.0f,1.0f}, model_.GetUvHandle());
	}

	// map中心点描画
	//GameObject::Draw(worldTransform_);

	/*
	// 壁描画
	for (auto& world : WallWorlds_) {
		GameObject::Draw(*world);
	}
	*/

	/*
	for (auto& collider : colliders_) {
		collider->Draw();
	}
	*/
}

void Map::DrawSprite() {
	//マップ編集モード
#ifdef _DEBUG
	if (isEditOn_) {
		editCursor_->Draw();
	}
#endif // _DEBUG
}

void Map::Finalize() {

}

const std::vector<Collider*> Map::GetWallCollider() {
	std::vector<Collider*> colliders;

	int32_t location = 0;

	for (int32_t y = 0; y < mapData_.size(); y++) {
		for (int32_t x = 0; x < mapData_[y].size(); x++) {
			if (mapData_[y][x] == Block) {

				colliders.push_back(colliders_[location].get());

			}
			location++;
		}
	}

	return colliders;
	// TODO: return ステートメントをここに挿入します
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

void Map::ImGuiDraw() {
#ifdef _DEBUG
	ImGui::Begin("map");
	ImGui::DragFloat3("map world pos", &worldTransform_.translation_.x, 0.1f);
	ImGui::DragFloat3("rotate", &worldTransform_.rotation_.x, 0.1f);
	ImGui::DragFloat3("map world scale", &worldTransform_.scale_.x, 0.1f);
	ImGui::Checkbox("is edit on", &isEditOn_);
	ImGui::End();
#endif // _DEBUG
}

void Map::MapPositioningInitialize() {


	//xに一番多いサイズ取得
	for (auto& mapdataX : mapData_) {
		if (mapTileNumX_ < (int)mapdataX.size()) {
			mapTileNumX_ = (int)mapdataX.size();
		}
	}
	mapTileNumY_ = (int)mapData_.size();


	// 回転中心点の検索
	///（マップの数）ｘ（半径）の半分　-　位置ブロックの半径の半分
	Vector3 centerPos = { (mapTileNumX_ * tileWide_) / 2 - (tileWide_ / 2), -(mapTileNumY_ * tileWide_) / 2 + (tileWide_ / 2), 0 };

	worldTransform_.Initialize();
	worldTransform_.translation_ = centerPos;
	worldTransform_.UpdateMatrix();

	//各種ベクター初期化
	WallWorlds_.clear();
	colliders_.clear();
	boxWorlds_.clear();

	//Collider初期化のための変数
	uint32_t colliderAcceces = 0;
	int containerNumberY = 0;
	for (auto& mapDataY : mapData_) {
		int containerNumberX = 0;
		for (auto& mapDataX : mapDataY) {

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

			WallWorlds_.emplace_back(std::move(world));
			//Collider

			std::unique_ptr<Collider> collider;
			collider = std::make_unique<Collider>();

			collider->Initialize(WallWorlds_[colliderAcceces].get(), "blockTile", viewProjection_, directionalLight_);
			colliders_.push_back(std::move(collider));


			colliderAcceces++;


			if (mapDataX == Box) {

				std::unique_ptr<WorldTransform> Bworld;
				Bworld = std::make_unique<WorldTransform>();
				Bworld->Initialize();
				Bworld->translation_ = { tileWide_ * containerNumberX, -tileWide_ * containerNumberY, 0 };

				// 親子関係設定
				Bworld->SetParent(&worldTransform_);

				// ベクトル差分を代入
				Vector3 subPos = Subtract(Bworld->translation_, worldTransform_.translation_);

				// 座標代入
				Bworld->translation_ = subPos;

				Bworld->UpdateMatrix();

				boxWorlds_.emplace_back(std::move(Bworld));
			}else if (mapDataX == Player) {

				playerWorld_.Initialize();

				playerWorld_.translation_ = { tileWide_ * containerNumberX, -tileWide_ * containerNumberY, 0 };
				playerWorld_.SetParent(&worldTransform_);

				playerWorld_.UpdateMatrix();
				// ベクトル差分を代入
				Vector3 subPos = Subtract(playerWorld_.translation_, worldTransform_.translation_);

				// 座標代入
				playerWorld_.translation_ = subPos;

				playerWorld_.UpdateMatrix();
			}else if (mapDataX == Goal) {
				//ゴールのWorldTransform設定
				goalW_.Initialize();

				goalW_.translation_={ tileWide_ * containerNumberX, -tileWide_ * containerNumberY, 0 };
				goalW_.SetParent(&worldTransform_);

				goalW_.UpdateMatrix();

				// ベクトル差分を代入
				Vector3 subPos = Subtract(goalW_.translation_, worldTransform_.translation_);

				// 座標代入
				goalW_.translation_ = subPos;

				goalW_.UpdateMatrix();
			}

			containerNumberX++;
		}
		containerNumberY++;
	}
}

void SaveMapData(const char* fileName, std::vector<std::vector<int>> datas) {
	//ファイルの読み込み
	FILE* fp;
	int err = fopen_s(&fp, fileName, "w");


	//読み込めなかったら返す
	if (err != 0) {

		assert("%s は見つかりませんでした!zako!");
	}
	else {
		for (auto& data : datas) {
			fprintf(fp, "{");
			for (auto& indata : data) {
				fprintf(fp, "%d , ", indata);
			}
			fprintf(fp, "},\n");
		}


	}

	//ファイルを閉じる
	fclose(fp);
}

void Map::MapEditor(const ViewProjection& view) {

#ifdef _DEBUG

	if (isEditOn_) {

		ImGui::Begin("block edit");
		ImGui::Text("0 = Air : 1 = Block : 2 = PlayerSpawn");
		ImGui::Text("Move : Arrow Key");
		if (ImGui::Button("SaveMap")) {
			SaveMapData(map1Pass[mapPassNumber_], mapData_);
			MessageBoxA(nullptr, "SaveComplete!", "mapData", 0);

			isInitializeEditMode_ = true;
		}

		if (ImGui::Button("LoadMap")) {
			mapData_ = LoadMapData(map1Pass[mapPassNumber_]);
			isInitializeEditMode_ = true;
			MessageBoxA(nullptr, "TextLoaded!", "mapData", 0);
		}

		ImGui::End();


		//初期化していなければ初期化
		if (!isInitializeEditMode_) {

			//フラグON
			isInitializeEditMode_ = true;

			//座標取得
			Vector3 reticlePosV3 = MakeTranslation(WallWorlds_[0]->matWorld_);

			Matrix4x4 matviewport = MakeViewportMatrix(0, 0, WinApp::kWindowWidth, WinApp::kWindowHeight, 0, 1);

			Matrix4x4 matVPV = view.matView * view.matProjection * matviewport;

			reticlePosV3 = Transform(reticlePosV3, matVPV);

			//V2に変換
			reticlePos_ = { reticlePosV3.x,reticlePosV3.y };

			//画像読み込み
			cursorTex_ = TextureManager::Load("editcursor.png");

			//テクスチャクラス初期化
			editCursor_.reset(Sprite::Create(cursorTex_, reticlePos_));
		}
		else {

#pragma region カーソル移動処理
			//移動入力受付
			Vector2 move = { 0,0,0 };
			if (input_->TriggerKey(DIK_UP)) {
				move.y--;
			}
			if (input_->TriggerKey(DIK_DOWN)) {
				move.y++;
			}
			if (input_->TriggerKey(DIK_LEFT)) {
				move.x--;
			}
			if (input_->TriggerKey(DIK_RIGHT)) {
				move.x++;
			}

			//コンテナをつかしたかのフラグ
			bool isAddContainer = false;

			//配列外参照対策処理(Y)
			referenceMapY_ += (int)move.y;
			if (referenceMapY_ < 0) {
				referenceMapY_ = 0;
			}
			else if (referenceMapY_ >= mapData_.size()) {
				//最後尾のmapdataをコピーして新しく作成
				size_t maxsize = mapData_.size() - (size_t)1;
				mapData_.emplace_back(mapData_[maxsize]);
				isAddContainer = true;
			}

			//配列がい参照対策（X）
			referenceMapX_ += (int)move.x;
			if (referenceMapX_ < 0) {
				referenceMapX_ = 0;
			}
			else if (referenceMapX_ >= mapData_[referenceMapY_].size()) {

				while (true) {

					if (referenceMapX_ < mapData_[referenceMapY_].size()) {
						break;
					}


					//最後尾に0を追加して終わる
					mapData_[referenceMapY_].emplace_back(1);
					isAddContainer = true;
				}

			}

			//MAPData変更による各種初期化
			if (isAddContainer) {
				MapPositioningInitialize();
			}



			Vector3 reticlePosV3 = { 0,0,0 };

			//座標の取得
			int worldLocation = 0;
			for (int32_t y = 0; y < mapData_.size(); y++) {

				for (int32_t x = 0; x < mapData_[y].size(); x++) {

					if (y == referenceMapY_ && x == referenceMapX_) {
						//座標取得
						reticlePosV3 = MakeTranslation(WallWorlds_[worldLocation]->matWorld_);

					}

					worldLocation++;
				}

			}

#pragma endregion

#pragma region ブロックの置き換え
			{
				int pickChip = mapData_[referenceMapY_][referenceMapX_];
				if (input_->TriggerKey(DIK_0)) {
					pickChip = 0;
				}
				if (input_->TriggerKey(DIK_1)) {
					pickChip = 1;
				}
				if (input_->TriggerKey(DIK_2)) {
					pickChip = 2;
				}


				mapData_[referenceMapY_][referenceMapX_] = pickChip;
			}
#pragma endregion





			Matrix4x4 matviewport = MakeViewportMatrix(0, 0, WinApp::kWindowWidth, WinApp::kWindowHeight, 0, 1);

			Matrix4x4 matVPV = view.matView * view.matProjection * matviewport;

			reticlePosV3 = Transform(reticlePosV3, matVPV);

			//V2に変換
			reticlePos_ = { reticlePosV3.x,reticlePosV3.y };
			editCursor_->position_ = reticlePos_;
		}

	}
#endif // _DEBUG


}

float EsingFloat(const float t, const float start, const float end) {
	return start * (1.0f - t) + end * t;
}


#pragma region State初期化
void Map::InitializeStateNormal() { isRotating = false; rotateComplete = true; }

void Map::InitializeStateRightRotation() {
	isRotating = true;

	// 開始回転と終了回転設定
	rotateE_ = { .startT{worldTransform_.rotation_.z},
		.endT{worldTransform_.rotation_.z - HalfPI_} };

	t_ = 0;
}

void Map::InitializeStateLeftRotation() {
	isRotating = true;

	// 開始回転と終了回転設定
	rotateE_ = { .startT{worldTransform_.rotation_.z}, .endT{worldTransform_.rotation_.z + HalfPI_} };

	t_ = 0;
}

#pragma endregion

#pragma region State更新

void Map::UpdateStateNormal() {

	if (input_->PushKey(DIK_E)) {
		stateRequest_ = State::kRightRotation;
		
	}
	if (input_->PushKey(DIK_Q)) {
		stateRequest_ = State::kLeftRotation;
	}
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
#pragma endregion



