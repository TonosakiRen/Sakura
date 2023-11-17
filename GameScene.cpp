#include "GameScene.h"
#include "externals/imgui/imgui.h"
#include <cassert>

using namespace DirectX;

void (GameScene::* GameScene::SceneUpdateTable[])() = {
	&GameScene::TitleUpdate,
	&GameScene::InGameUpdate,
};

void (GameScene::* GameScene::SceneInitializeTable[])() = {
	&GameScene::TitleInitialize,
	&GameScene::InGameInitialize,
};


GameScene::GameScene() {};

GameScene::~GameScene() {};

void GameScene::Initialize() {
	dxCommon_ = DirectXCommon::GetInstance();
	input_ = Input::GetInstance();

	viewProjection_.Initialize();
	viewProjection_.translation_.y = 7.0f;
	viewProjection_.translation_.z = -43.0f;


	directionalLight_.Initialize();
	directionalLight_.direction_ = { 1.0f, -1.0f, 1.0f };
	directionalLight_.UpdateDirectionalLight();


	textureHandle_ = TextureManager::Load("uvChecker.png");

	sprite_.reset(Sprite::Create(textureHandle_, { 0.0f,0.0f }));

	sphere_ = std::make_unique<GameObject>();
	sphere_->Initialize("sphere", &viewProjection_, &directionalLight_);

	//マップの壁
	map_ = std::make_unique<Map>();
	map_->Initialize("stage", &viewProjection_, &directionalLight_, mapPassNum_);

	player_ = std::make_unique<Player>();
	player_->Initialize("player", &viewProjection_, &directionalLight_, map_->GetClearW());


	//箱の初期化
	int managementNum = 0;
	for (auto& world : map_->GetBoxWorldTransform()) {
		std::unique_ptr<Box>box = std::make_unique<Box>();
		box->Initialize("box", &viewProjection_, &directionalLight_, *world.get(), managementNum);

		boxes_.emplace_back(std::move(box));

		managementNum++;
	}


	clearBox_ = std::make_unique<ClearBox>();
	clearBox_->Initialize("player", &viewProjection_, &directionalLight_, map_->GetPlayerW());

}

void GameScene::Update() {
	//camera light
	{
		//カメラの座標をマップの中心点に合わせる
		Vector3 mapCenterV3 = map_->GetMapCenter();
		viewProjection_.translation_.x = mapCenterV3.x;
		viewProjection_.translation_.y = mapCenterV3.y;
		float mostlength = 0.0f;
		if (viewProjection_.translation_.x > -viewProjection_.translation_.y) {
			mostlength = viewProjection_.translation_.x;
		}
		else {
			mostlength = -viewProjection_.translation_.y;
		}
		viewProjection_.translation_.z = -43.0f - (mostlength - 10.0f) * 2.0f;

		// camera
		viewProjection_.DebugMove();
		viewProjection_.UpdateMatrix();

		// light
		ImGui::DragFloat3("light", &directionalLight_.direction_.x, 0.01f);
		ImGui::DragFloat4("lightcolor", &directionalLight_.color_.x, 0.01f);
		directionalLight_.direction_ = Normalize(directionalLight_.direction_);
		directionalLight_.UpdateDirectionalLight();
	}
	Collider::SwitchIsDrawCollider();

	//Scene初期化
	if (sceneRequest_) {
		scene_ = sceneRequest_.value();
		(this->*SceneInitializeTable[static_cast<size_t>(scene_)])();
		sceneRequest_ = std::nullopt;
	}
	//SceneUpdate
	(this->*SceneUpdateTable[static_cast<size_t>(scene_)])();



}

void GameScene::TitleInitialize() {

}
void GameScene::TitleUpdate() {

	if (input_->TriggerKey(DIK_P)) {
		sceneRequest_ = Scene::InGame;
	}

}
void GameScene::InGameInitialize() {
	map_->Initialize("stage", &viewProjection_, &directionalLight_, mapPassNum_);

	player_->Initialize("player", &viewProjection_, &directionalLight_, map_->GetPlayerW());

	//箱の初期化
	int managementNum = 0;
	for (auto& world : map_->GetBoxWorldTransform()) {
		std::unique_ptr<Box>box = std::make_unique<Box>();
		box->Initialize("box", &viewProjection_, &directionalLight_, *world.get(), managementNum);

		boxes_.emplace_back(std::move(box));

		managementNum++;
	}

	clearBox_->Initialize("player", &viewProjection_, &directionalLight_, map_->GetClearW());

	//次イニシャライズするときに
	mapPassNum_++;
}



void GameScene::InGameUpdate() {
	if (input_->TriggerKey(DIK_P)) {
		sceneRequest_ = Scene::Title;
	}

	map_->Update();
	map_->MapEditor(viewProjection_);

	clearBox_->Update();

	player_->Update();

	for (auto& box : boxes_) {
		box->Update();
	}

	//マップが回転していないときのみコリジョン処理
	if (!Map::isRotating) {
		AllCollision();
	}

	//条件を満たしている場合のシーンチェンジ
	InGameSceneChange();
}


void GameScene::AllCollision() {

	//ブロックとの押し出し処理
	for (auto& wall : map_->GetWallCollider()) {
		player_->Collision(*wall);
	}


	//ボックス処理
	for (auto& box : boxes_) {

		//先にプレイヤーにあわせて押し出し
		box->Collision(player_->collider_);


		/*
		//二個目のボックス処理
		for (auto& box2 : boxes_) {
			if (box2->GetMaagementNum() != box->GetMaagementNum()) {
				box2->Collision(*box->GetCollider());
			}

			for (auto& wall : map_->GetWallCollider()){
				box2->Collision(*wall);
				//ボックスの下にあるコライダーが浮いているか否か
				box2->CollisionUnderCollider(*wall);

				box->Collision(*wall);
				//ボックスの下にあるコライダーが浮いているか否か
				box->CollisionUnderCollider(*wall);

			}

			//押し出された箱の処理
			box->Collision(*box2->GetCollider());
		}
		*/


		//壁チップ検索
		for (auto& wall : map_->GetWallCollider()) {
			//プレイヤーで埋まっていたまたは元々埋まっていた場合押し出し処理
			box->Collision(*wall);
		}


		//押し戻しによって返された分プレイヤーも返す
		player_->Collision(*box->GetCollider());
	}

	/*
	//ブロックとの押し出し処理
	for (auto& wall : map_->GetWallCollider()) {
		player_->Collision(*wall);
	}
	*/
#pragma region プレイヤーの状態更新

	player_->UnderColliderUpdate();


	if (!player_->CheckStateSame(PlayerState::kNormal)) {
		//壁との処理
		for (auto& wall : map_->GetWallCollider()) {
			//下のコライダーとの当たり判定処理
			if (player_->IsUnderColliderCollision(*wall)) {

				//ぴったりくっついている場合
				if (player_->IsSetPerfect(*wall)) {

					//状態変更（ノーマル
					player_->SetState(PlayerState::kNormal);

					break;
				}

			}
		}
	}
#pragma endregion



	//ボックスの状態変化
	for (auto& box : boxes_) {

		bool isBoxHit = false;

		for (auto& wall : map_->GetWallCollider()) {
			//ボックスの下にあるコライダーが浮いているか否か
			if (box->CollisionUnderCollider(*wall)) {

				if (!isBoxHit) {
					if (box->IsSetPerfect(*wall)) {
						//状態を変更
						box->SetState(kStay);
						isBoxHit = true;
					}
				}
			}
		}

		//当たっていなかったので変更
		if (!isBoxHit) {
			box->SetState(kFall);
		}
	}




	//クリアボックスとプレイヤーとの当たり判定
	if (clearBox_->IsHitCollision(player_->collider_)) {
		//シーン変更フラグをON
		isSceneChange_ = true;
	}

}

void GameScene::InGameSceneChange() {

	if (isSceneChange_) {
		//シーンを変更
		//sceneRequest_ = Scene::InGame;
	}
}

void GameScene::ModelDraw() {
	switch (scene_) {
	case GameScene::Scene::Title:
		break;
	case GameScene::Scene::InGame:
		map_->Draw();
		clearBox_->Draw();
		player_->Draw();
		for (auto& box : boxes_) {
			box->Draw();
		}
		break;
	default:
		break;
	}

}

void GameScene::ParticleDraw() {
	switch (scene_) {
	case GameScene::Scene::Title:
		break;
	case GameScene::Scene::InGame:
		break;
	default:
		break;
	}
}

void GameScene::ParticleBoxDraw() {
	switch (scene_) {
	case GameScene::Scene::Title:
		break;
	case GameScene::Scene::InGame:
		break;
	default:
		break;
	}
}

void GameScene::PreSpriteDraw() {
	switch (scene_) {
	case GameScene::Scene::Title:
		break;
	case GameScene::Scene::InGame:
		break;
	default:
		break;
	}
}

void GameScene::PostSpriteDraw() {
	switch (scene_) {
	case GameScene::Scene::Title:
		break;
	case GameScene::Scene::InGame:
		map_->DrawSprite();
		break;
	default:
		break;
	}
}

void GameScene::Draw(CommandContext& commandContext) {
	// 背景スプライト描画
	Sprite::PreDraw(commandContext);
	PreSpriteDraw();
	Sprite::PostDraw();

	DirectXCommon::GetInstance()->ClearMainDepthBuffer();

	//3Dオブジェクト描画
	Model::PreDraw(commandContext);
	ModelDraw();
	Model::PostDraw();

	//Particle描画
	Particle::PreDraw(commandContext);
	ParticleDraw();
	Particle::PostDraw();

	//Particle描画
	ParticleBox::PreDraw(commandContext);
	ParticleBoxDraw();
	ParticleBox::PostDraw();
}

void GameScene::UIDraw(CommandContext& commandContext) {
	// 前景スプライト描画
	Sprite::PreDraw(commandContext);
	PostSpriteDraw();
	Sprite::PostDraw();
}