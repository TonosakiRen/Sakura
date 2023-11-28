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
	player_->Initialize("player", &viewProjection_, &directionalLight_, map_->GetPlayerW());


	//箱の初期化
	int managementNum = 0;
	for (auto& world : map_->GetBoxWorldTransform()) {
		std::unique_ptr<Box>box = std::make_unique<Box>();
		box->Initialize("box", &viewProjection_, &directionalLight_, *world.get(), managementNum);

		boxes_.emplace_back(std::move(box));

		managementNum++;
	}

	mapPassNum_++;

	deadParticle_ = std::make_unique<DeadLineParticle>();
	deadParticle_->Initialize({ 0.0f,0.0f }, { 1.0f,0.0f });


	clearBox_ = std::make_unique<ClearBox>();
	clearBox_->Initialize("clearBox", &viewProjection_, &directionalLight_, map_->GetClearW());

	bikkuri_ = TextureManager::Load("!.png");

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

	deadParticle_->SetMapCenter(map_->GetMapCenter());
	deadParticle_->Update();


}

void GameScene::TitleInitialize() {

}
void GameScene::TitleUpdate() {

	if (input_->TriggerKey(DIK_P)) {
		sceneRequest_ = Scene::InGame;
	}

}
void GameScene::InGameInitialize() {

}

void GameScene::StageInitialize(int stageNum)
{
	if (mapPassNum_ >= 6) {
		mapPassNum_ = 0;
	}

	map_->StageInitialize(stageNum);
	player_->StageInitialize(map_->GetPlayerW());
	clearBox_->StageInitialize(map_->GetClearW());
	boxes_.clear();
	int managementNum = 0;
	for (auto& world : map_->GetBoxWorldTransform()) {
		std::unique_ptr<Box>box = std::make_unique<Box>();
		box->Initialize("box", &viewProjection_, &directionalLight_, *world.get(), managementNum);

		boxes_.emplace_back(std::move(box));

		managementNum++;
	}

	//次イニシャライズするときに
	mapPassNum_++;
	
}


void GameScene::InGameUpdate() {
	if (input_->TriggerKey(DIK_P)) {
		sceneRequest_ = Scene::Title;
	}

	if (input_->TriggerKey(DIK_2)) {
		isStageChange_ = true;
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
		AllCollisionPrePosUpdate();
	}

	//ボックスの枠外落下処理
	CheckBoxDead();

	//条件を満たしている場合のシーンチェンジ
	InGameSceneChange();
}


void GameScene::AllCollision() {


#pragma region 押し戻し処理
	//ブロックとの押し出し処理
	for (auto& wall : map_->GetWallCollider()) {
		player_->Collision(*wall);


		for (auto& box : boxes_) {
			if (!box->GetIsDead()) {
				box->Collision(*wall);
			}
		}
	}


	//ボックス処理
	for (auto& box : boxes_) {
		if (!box->GetIsDead()) {
			//先にプレイヤーにあわせて押し出し
			box->Collision(player_->collider_);


			//二個目のボックス処理
			for (auto& box2 : boxes_) {
				//一個目をもとに押し戻し処理
				if (box2->GetMaagementNum() != box->GetMaagementNum()) {
					box2->Collision(*box->GetCollider());


					//押し出しによって壁に埋まったら押し出す
					for (auto& wall : map_->GetWallCollider()) {
						box2->Collision(*wall);
					}


					for (auto& box3 : boxes_) {
						if (box3->GetMaagementNum() != box2->GetMaagementNum()) {
							box3->Collision(*box2->GetCollider());

							//押し出しによって壁に埋まったら押し出す
							for (auto& wall : map_->GetWallCollider()) {
								box3->Collision(*wall);
							}

							//押し出された箱２に押し戻された処理
							box2->Collision(*box3->GetCollider());
						}
					}


					//押し出された箱２に押し戻された処理
					box->Collision(*box2->GetCollider());
				}
			}



			//壁チップ検索
			for (auto& wall : map_->GetWallCollider()) {
				//プレイヤーで埋まっていたまたは元々埋まっていた場合押し出し処理
				box->Collision(*wall);
			}


			//押し戻しによって返された分プレイヤーも返す
			player_->Collision(*box->GetCollider());
		}
	}

#pragma endregion

#pragma region プレイヤーの状態変化
	bool isActivate_ = false;

	//壁との処理
	for (auto& wall : map_->GetWallCollider()) {
		//下のコライダーとの当たり判定処理
		if (player_->IsUnderColliderCollision(*wall)) {

			//ぴったりくっついている場合
			if (player_->IsSetPerfect(*wall)) {

				//状態変更（ノーマル
				player_->SetState(PlayerState::kNormal);
				isActivate_ = true;
				break;
			}

		}
	}

	//ボックスとの処理
	for (auto& box : boxes_) {
		if (!box->GetIsDead()) {
			//下のコライダーとの当たり判定処理
			if (player_->IsUnderColliderCollision(*box->GetCollider())) {

				//ぴったりくっついている場合
				if (player_->IsSetPerfect(*box->GetCollider())) {

					//状態変更（ノーマル
					player_->SetState(PlayerState::kNormal);
					isActivate_ = true;
					break;
				}
			}

			if (!player_->CheckStateReqest()) {
				//Boxが縦向きの時の処理
				if (player_->CheckBoxStateSame(RectangleFacing::kPortrait)) {
					//上のコライダーとの当たり判定
					if (player_->IsUpColliderCollision(*box->GetCollider())) {
						//ぴったりくっついている場合&&地面にいる場合
						if (player_->IsSetPerfect(*box->GetCollider()) && player_->CheckStateSame(PlayerState::kNormal)) {


							//状態変更
							player_->SetBoxState(RectangleFacing::kLandscape);
							isActivate_ = true;
							break;
						}
					}
				}
			}
		}
	}


	if (!isActivate_) {
		player_->SetStateNoInitialize(PlayerState::kJump);
	}
#pragma endregion

#pragma region ボックスの状態変化
	//
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

		for (auto& box2 : boxes_) {
			if (!box2->GetIsDead()) {
				//ボックスの下にあるコライダーが浮いているか否か
				if (box->CollisionUnderCollider(*box2->GetCollider())) {

					if (!isBoxHit) {
						if (box->IsSetPerfect(*box2->GetCollider())) {
							//状態を変更
							box->SetState(kStay);
							isBoxHit = true;
						}
					}
				}
			}
		}

		//当たっていなかったので変更
		if (!isBoxHit) {
			box->SetState(kFall);
		}
	}
#pragma endregion



	//ボックスがすべてない状態の時
	if (isHitClearBox_) {
		//クリアボックスとプレイヤーとの当たり判定&&長方形の向き状態が同じ
		if (clearBox_->IsHitCollision(player_->collider_)&&player_->GetRectangle()==clearBox_->GetRectangle()) {
			//シーン変更フラグをON
			isStageChange_ = true;
		}
	}

	//プレイヤーが死んだら初期化
	if (player_->GetIsDead()) {
		//シーン変更フラグをON
		StageInitialize(mapPassNum_ - 1);
	}
}

void GameScene::AllCollisionPrePosUpdate()
{
	//ブロックとの押し出し処理
	for (auto& wall : map_->GetWallCollider()) {
		wall->prePosUpdate();
	}
	player_->collider_.prePosUpdate();
	for (auto& box : boxes_) {
		box->GetCollider()->prePosUpdate();
	}

}

void GameScene::CheckBoxDead() {
	//仮で-50以下で消滅するように
	float deadLine = -50;

	//死んだ数チェック
	int deadNum_ = 0;

	for (auto& box : boxes_) {
		if (box->GetIsDead()) {
			deadNum_++;
		}else if (box->GetWorldTransform()->matWorld_.m[3][1]<=deadLine) {
			//死亡判定渡し
			box->SetIsDead(true);
		}

		
	}

	//すべて死んでいる場合
	if (deadNum_ == boxes_.size()) {
		isHitClearBox_ = true;
	}
}

void GameScene::InGameSceneChange() {

	if (isStageChange_) {
		StageInitialize(mapPassNum_);
		isStageChange_ = false;
	}
}

void GameScene::ModelDraw() {
	switch (scene_) {
	case GameScene::Scene::Title:
		break;
	case GameScene::Scene::InGame:

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
		deadParticle_->SetIsEmit(true);
		/*deadParticle_->emitterWorldTransform_.translation_.x = map_->GetMapCenter().x;
		deadParticle_->emitterWorldTransform_.translation_.y = 8.0f;*/

		/*deadParticle_->emitterWorldTransform_.translation_.y = -27.0f;*/
		deadParticle_->emitterWorldTransform_.scale_ = { 1.5f,1.5f,1.5f };
		deadParticle_->Draw(&viewProjection_,{1.0f,1.0f,1.0f,1.0f}, bikkuri_);
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
		map_->Draw();
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