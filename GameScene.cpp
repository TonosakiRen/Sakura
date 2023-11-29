#include "GameScene.h"
#include "externals/imgui/imgui.h"
#include <cassert>
#include "Easing.h"
#include "Audio.h"

using namespace DirectX;

bool GameScene::shutDown = false;

void (GameScene::* GameScene::SceneUpdateTable[])() = {
	&GameScene::TitleUpdate,
	&GameScene::InGameUpdate,
	&GameScene::ClearUpdate
};

void (GameScene::* GameScene::SceneInitializeTable[])() = {
	&GameScene::TitleInitialize,
	&GameScene::InGameInitialize,
	&GameScene::ClearInitialize
};


GameScene::GameScene() {};

GameScene::~GameScene() {};

void GameScene::Initialize() {
	dxCommon_ = DirectXCommon::GetInstance();
	input_ = Input::GetInstance();
	audio_ = Audio::GetInstance();

	viewProjection_.Initialize();
	viewProjection_.translation_.y = 7.0f;
	viewProjection_.translation_.z = -43.0f;
	viewProjection_.target_.x = -90.0f;


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

	playerAnimation_ = std::make_unique<PlayerAnimation>();
	playerAnimation_->SetPlayer(player_.get());

	map_->SetPlayer(player_.get());


	for (int i = 0; i < map_->GetMaxTile(); i++) {
		WorldTransform world;
		std::unique_ptr<Box>box = std::make_unique<Box>();
		box->Initialize("box", &viewProjection_, &directionalLight_, world, i);
		box->SetIsDead(true);
		boxes_.emplace_back(std::move(box));
	}
	isStageSelect_ = true;

	spawnBoxNum = 0;
	//ボックスの配置
	for (auto& world : map_->GetBoxWorldTransform()) {
		for (auto& box : boxes_) {
			if (box->GetIsDead()) {
				box->SetStartData(*world);
				spawnBoxNum++;
				break;
			}
		}
	}

	int SelectBoxCreatingCount = 0;
	int flystageNum = Map::maxMapNum_ - 1;
	for (auto& stageW : map_->GetStageSelectBox()) {
		std::unique_ptr<SelectStage>ssnew = std::make_unique<SelectStage>();

		if (SelectBoxCreatingCount <= 5) {
			ssnew->Initialize("gate", &viewProjection_, &directionalLight_, *stageW, flystageNum);
			flystageNum--;
		}
		else {
			ssnew->Initialize("gate", &viewProjection_, &directionalLight_, *stageW, flystageNum - 3);
			flystageNum++;
		}

		selectStage_.emplace_back(std::move(ssnew));

		SelectBoxCreatingCount++;
	}

	mapPassNum_++;

	deadParticle_ = std::make_unique<DeadLineParticle>();
	deadParticle_->Initialize({ 0.0f,0.0f }, { 1.0f,0.0f });


	clearBox_ = std::make_unique<ClearBox>();
	clearBox_->Initialize("gate", &viewProjection_, &directionalLight_, map_->GetClearW());
	player_->goalPos_ = clearBox_->GetWorldTransform()->translation_;

	bikkuri_ = TextureManager::Load("!.png");

	title_ = std::make_unique<GameObject>();
	title_->Initialize("title", &viewProjection_, &directionalLight_);
	title_->SetRotation({ Radian(90.0f),Radian(180.0f),0.0f });

	
	uint32_t halfBlackHandle = TextureManager::Load("halfBlack.png");
	halfBlack_.reset(Sprite::Create(halfBlackHandle, { WinApp::kWindowWidth / 2.0f ,WinApp::kWindowHeight / 2.0f }));
	halfBlack_->size_ = { WinApp::kWindowWidth ,WinApp::kWindowHeight };

	uint32_t gameloseHandle = TextureManager::Load("game_close.png");
	uint32_t titleSelectHandle = TextureManager::Load("titleSelect.png");
	uint32_t stageSelectHandle = TextureManager::Load("stageselect.png");
	uint32_t selectHandle = TextureManager::Load("select.png");

	gameCloseSprite_.reset(Sprite::Create(gameloseHandle, { WinApp::kWindowWidth / 2.0f ,WinApp::kWindowHeight / 2.0f + 150.0f + 216.0f }));
	stageSelectSprite_.reset(Sprite::Create(stageSelectHandle, { WinApp::kWindowWidth / 2.0f ,WinApp::kWindowHeight / 2.0f + 150.0f + 108.0f }));
	titleSelectSprite_.reset(Sprite::Create(titleSelectHandle, { WinApp::kWindowWidth / 2.0f ,WinApp::kWindowHeight / 2.0f + 150.0f }));
	selectSprite_.reset(Sprite::Create(selectHandle, { WinApp::kWindowWidth / 2.0f - 226.0f - 30.0f ,WinApp::kWindowHeight / 2.0f + 150.0f }));

	uint32_t pushAHandle = TextureManager::Load("pushA.png");

	uint32_t moveHandle = TextureManager::Load("move.png");
	uint32_t jumpHandle = TextureManager::Load("jump.png");
	uint32_t rotateHandle = TextureManager::Load("rotate.png");
	uint32_t dropHandle = TextureManager::Load("drop.png");

	pushASprite_.reset(Sprite::Create(pushAHandle, { WinApp::kWindowWidth / 2.0f, 900.0f}));

	rotateSprite_.reset(Sprite::Create(rotateHandle, { 1636.0f,244.0f }));
	moveSprite_.reset(Sprite::Create(moveHandle, { 272.0f,222.0f }));
	jumpSprite_.reset(Sprite::Create(jumpHandle, { 272.0f,611.0f }));
	dropSprite_.reset(Sprite::Create(dropHandle, { 1006.0f,244.0f }));

	uint32_t pauseHandle = TextureManager::Load("pause.png");
	pauseSprite_.reset(Sprite::Create(pauseHandle,{1862.0f,1015.0f}));

	size_t bgmHandle = audio_->SoundLoadWave("music.wav");
	size_t bgmPlayHandle = audio_->SoundPlayLoopStart(bgmHandle);
	audio_->SetValume(bgmPlayHandle, 0.1f);

	uint32_t numHandle = TextureManager::Load("1.png");
	oneSprite_.reset(Sprite::Create(numHandle));
	numHandle = TextureManager::Load("2.png");
	twoSprite_.reset(Sprite::Create(numHandle));
	numHandle = TextureManager::Load("3.png");
	threeSprite_.reset(Sprite::Create(numHandle));
	numHandle = TextureManager::Load("4.png");
	fourSprite_.reset(Sprite::Create(numHandle));
	numHandle = TextureManager::Load("5.png");
	fiveSprite_.reset(Sprite::Create(numHandle));
	 numHandle = TextureManager::Load("6.png");
	sixSprite_.reset(Sprite::Create(numHandle));
	 numHandle = TextureManager::Load("7.png");
	sevenSprite_.reset(Sprite::Create(numHandle));
	 numHandle = TextureManager::Load("8.png");
	eightSprite_.reset(Sprite::Create(numHandle));
	 numHandle = TextureManager::Load("9.png");
	nineSprite_.reset(Sprite::Create(numHandle));
	 numHandle = TextureManager::Load("10.png");
	tenSprite_.reset(Sprite::Create(numHandle));

	
	for (int i = 0; i < 10;i++) {
		if (i == 0) {
			oneSprite_->position_ = viewProjection_.MakeScreenVector(MakeTranslation(selectStage_[i]->GetWorldTransform()->matWorld_));
		}
		else if (i == 1) {
			twoSprite_->position_ = viewProjection_.MakeScreenVector(MakeTranslation(selectStage_[i]->GetWorldTransform()->matWorld_));
		}
		else if (i == 2) {
			threeSprite_->position_ = viewProjection_.MakeScreenVector(MakeTranslation(selectStage_[i]->GetWorldTransform()->matWorld_));
		}
		else if (i == 3) {
			fourSprite_->position_ = viewProjection_.MakeScreenVector(MakeTranslation(selectStage_[i]->GetWorldTransform()->matWorld_));
		}
		else if (i == 4) {
			fiveSprite_->position_ = viewProjection_.MakeScreenVector(MakeTranslation(selectStage_[i]->GetWorldTransform()->matWorld_));
		}
		else if (i == 5) {
			sixSprite_->position_ = viewProjection_.MakeScreenVector(MakeTranslation(selectStage_[i]->GetWorldTransform()->matWorld_));
		}
		else if (i == 6) {
			sevenSprite_->position_ = viewProjection_.MakeScreenVector(MakeTranslation(selectStage_[i]->GetWorldTransform()->matWorld_));
		}
		else if (i == 7) {
			eightSprite_->position_ = viewProjection_.MakeScreenVector(MakeTranslation(selectStage_[i]->GetWorldTransform()->matWorld_));
		}
		else if (i == 8) {
			nineSprite_->position_ = viewProjection_.MakeScreenVector(MakeTranslation(selectStage_[i]->GetWorldTransform()->matWorld_));
		}
		else if (i == 9) {
			tenSprite_->position_ = viewProjection_.MakeScreenVector(MakeTranslation(selectStage_[i]->GetWorldTransform()->matWorld_));
		}
		
	}
	
	
}

void GameScene::Update() {


	if (input_->TriggerKey(DIK_ESCAPE)) {
		shutDown = true;
	}

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

		title_->SetPosition(Vector3{ viewProjection_.translation_.x,viewProjection_.translation_.y + 10.0f,viewProjection_.translation_.z });

		

		float theta = (1.0f / 120.0f)*(float)std::numbers::pi*2.0f;


		titleAnimeT_ += theta;

		if (titleAnimeT_ >= (float)std::numbers::pi * 2.0f) {
			titleAnimeT_ -= (float)std::numbers::pi * 2.0f;
		}

		float a = 1 * std::sinf(titleAnimeT_);

		ChangeY_ = title_->GetWorldTransform()->translation_.z +a ;

		title_->GetWorldTransform()->translation_.z = ChangeY_;

#ifdef _DEBUG
		ImGui::Begin("title");
		ImGui::DragFloat3("pos", &title_->GetWorldTransform()->translation_.x);
		ImGui::End();
#endif // _DEBUG

		

		title_->Update();





		
		switch (inGameScene) {
		case GameScene::Title:

			if (!isPause_) {
				if (!isBackTitle) {
					if (input_->TriggerKey(DIK_T) || input_->TriggerButton(XINPUT_GAMEPAD_A)) {
						isTitleCameraMove = true;
						size_t handle = audio_->SoundLoadWave("Select.wav");
						size_t selectHandle = audio_->SoundPlayWave(handle);
						audio_->SetValume(selectHandle, 0.1f);
					}

					if (isTitleCameraMove) {
						viewProjection_.target_.x = Easing::easing(titleCameraT, Radian(-90.0f), 0.0f, 0.02f);
					}
					else {
						viewProjection_.target_.x = Radian(-90.0f);
					}

					if (titleCameraT >= 1.0f) {
						inGameScene = InGame;
						isTitleCameraMove = false;
						titleCameraT = 0.0f;
					}
				}
				else {
					viewProjection_.target_.x = Easing::easing(titleCameraT, 0.0f, Radian(-90.0f), 0.02f);
					if (titleCameraT >= 1.0f) {
						isBackTitle = false;
						titleCameraT = 0.0f;
					}
				}
			}

			break;
		case GameScene::InGame:
			viewProjection_.target_.x = 0.0f;
			break;

		default:
			break;
		}

		// camera
		viewProjection_.DebugMove();
		viewProjection_.UpdateMatrix();
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

}

void GameScene::TitleInitialize() {
	
}
void GameScene::TitleUpdate() {

	
	//シーン移動
	if (input_->TriggerKey(DIK_P)) {
		sceneRequest_ = Scene::InGame;
		mapPassNum_ = 0;
	}

	//ゲームループを抜ける
	if (input_->TriggerKey(DIK_ESCAPE)) {

	}



}
void GameScene::InGameInitialize() {

}

void GameScene::StageInitialize(int stageNum) {
	if (mapPassNum_ >= 6) {
		//mapPassNum_ = 0;
	}
	sceneAnime_ = SceneAnimation::kInGame;

	map_->StageInitialize(stageNum);
	player_->StageInitialize(map_->GetPlayerW());
	clearBox_->StageInitialize(map_->GetClearW(), stageNum);
	player_->goalPos_ = clearBox_->GetWorldTransform()->translation_;
	playerAnimation_->Initialize();

	for (auto& box : boxes_) {
		box->SetIsDead(true);
	}


	spawnBoxNum = 0;
	//ボックスの配置
	for (auto& world : map_->GetBoxWorldTransform()) {
		for (auto& box : boxes_) {
			if (box->GetIsDead()) {
				box->SetStartData(*world);
				spawnBoxNum++;
				break;
			}
		}
	}


	//次イニシャライズするときに
	if (stageNum == mapPassNum_ - 1) {
		//プレイヤーが死んだ場合はここに入る
	}
	else {
		mapPassNum_++;
	}

	isHitClearBox_ = false;

	if (stageNum == 0) {
		isStageSelect_ = true;
	}
	else {
		isStageSelect_ = false;
	}

}


void GameScene::InGameUpdate() {

	if ((input_->TriggerKey(DIK_P) || input_->TriggerButton(XINPUT_GAMEPAD_START)) && isPause_ == false) {
		size_t handle = audio_->SoundLoadWave("Select.wav");
		size_t selectHandle = audio_->SoundPlayWave(handle);
		audio_->SetValume(selectHandle, 0.1f);
		isPause_ = true;
	}
	else
		if ((input_->TriggerKey(DIK_P) || input_->TriggerButton(XINPUT_GAMEPAD_START)) && isPause_ == true) {
			size_t handle = audio_->SoundLoadWave("Select.wav");
			size_t selectHandle = audio_->SoundPlayWave(handle);
			audio_->SetValume(selectHandle, 0.1f);
			isPause_ = false;
		}
	ImGui::Text("%d", pauseSelectNum_);

	if (isPause_) {

		if (input_->TriggerKey(DIK_DOWN) || input_->DownLStick()) {
			pauseSelectNum_++;
			size_t handle = audio_->SoundLoadWave("Select.wav");
			size_t selectHandle = audio_->SoundPlayWave(handle);
			audio_->SetValume(selectHandle, 0.1f);
		}
		if (input_->TriggerKey(DIK_UP) || input_->UpLStick()) {
			pauseSelectNum_--;
			size_t handle = audio_->SoundLoadWave("Select.wav");
			size_t selectHandle = audio_->SoundPlayWave(handle);
			audio_->SetValume(selectHandle, 0.1f);
		}
		pauseSelectNum_ = (int)clamp((float)pauseSelectNum_, 0, 2);

		if (input_->TriggerKey(DIK_SPACE) || input_->TriggerButton(XINPUT_GAMEPAD_A)) {
			size_t handle = audio_->SoundLoadWave("Select.wav");
			size_t selectHandle = audio_->SoundPlayWave(handle);
			audio_->SetValume(selectHandle, 0.1f);
			if (pauseSelectNum_ == 0) {
				if (inGameScene == InGame) {
					isBackTitle = true;
				}
				inGameScene = Title;
				StageInitialize(0);
				isPause_ = false;
			}
			else
				if (pauseSelectNum_ == 1) {
					if (inGameScene == Title) {
						isTitleCameraMove = true;
					}
					else {
						inGameScene = InGame;
						StageInitialize(0);
					}
					isPause_ = false;
				}
				else if (pauseSelectNum_ == 2) {
					shutDown = true;
				}

		}

	}
	else {
		pauseSelectNum_ = 0;

		
		switch (inGameScene) {
		case GameScene::Title:
			

			
			break;
		case GameScene::InGame:

			switch (sceneAnime_) {
			case GameScene::SceneAnimation::kStart:
				if (map_->StartAnimation()) {
					sceneAnime_ = SceneAnimation::kInGame;
				}
				map_->UpdateMatrix();
				player_->UpdateMatiries();

				if (!isStageSelect_) {
					clearBox_->UpdateMatirx();
				}



				for (auto& box : boxes_) {
					if (!box->GetIsDead()) {
						box->UpdateMatrix();
					}
				}
				break;
			case GameScene::SceneAnimation::kInGame:

				if (clearBox_->GetWorldTransform()) {
					clearBox_->UpdateMatirx();
				}
				for (auto& box : boxes_) {
					if (!box->GetIsDead()) {
						box->UpdateMatrix();
					}
				}
				map_->Update();
				map_->MapEditor(viewProjection_);

				clearBox_->Update();

				player_->Update();

				for (auto& box : boxes_) {
					if (!box->GetIsDead()) {
						box->Update();
					}
				}

				//マップが回転していないときのみコリジョン処理
				if (!Map::isRotating && player_->isGoal_ == false) {
					AllCollision();
					AllCollisionPrePosUpdate();
				}

				if (player_->isFinishGoalAnimation_ == true) {
					isStageChange_ = true;
				}



				playerAnimation_->Update();

				deadParticle_->Update();


				//ボックスの枠外落下処理
				CheckBoxDead();

				//条件を満たしている場合のシーンチェンジ
				InGameSceneChange();
				break;
			case GameScene::SceneAnimation::kEnd:
				if (map_->EndAnimation()) {
					NextScene();
				}
				map_->UpdateMatrix();
				player_->UpdateMatiries();
				clearBox_->UpdateMatirx();
				for (auto& box : boxes_) {
					if (!box->GetIsDead()) {
						box->UpdateMatrix();
					}
				}
				break;
			default:
				break;
			}

		}
	}

	//セレクトシーンだけ更新
	if (isStageSelect_) {
		for (auto& stagesele : selectStage_) {
			stagesele->Update();
		}
	}

}


void GameScene::AllCollision() {

	//回転後の更新処理が完全に終わったら処理
	if (!Map::rotateComplete && !Map::isRotating) {
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
					if (!box2->GetIsDead()) {
						//一個目をもとに押し戻し処理
						if (box2->GetMaagementNum() != box->GetMaagementNum()) {
							box2->Collision(*box->GetCollider());


							//押し出しによって壁に埋まったら押し出す
							for (auto& wall : map_->GetWallCollider()) {
								box2->Collision(*wall);
							}


							for (auto& box3 : boxes_) {
								if (!box3->GetIsDead()) {
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
							}


							//押し出された箱２に押し戻された処理
							box->Collision(*box2->GetCollider());
						}
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
					}
				}



			}
		}

		if (!isActivate_) {
			player_->SetStateNoInitialize(PlayerState::kJump);
		}


		for (auto& box : boxes_) {
			if (!box->GetIsDead()) {
				if (!player_->CheckStateReqest()) {
					//Boxが縦向きの時の処理
					if (player_->CheckBoxStateSame(RectangleFacing::kPortrait)) {
						//上のコライダーとの当たり判定&&下のコライダー反応なし
						if (player_->IsUpColliderCollision(*box->GetCollider())) {

							bool statechange_ = false;

							//ぴったりくっついている場合&&地面にいる場合
							if (player_->IsSetPerfect(*box->GetCollider()) && player_->CheckStateSame(PlayerState::kNormal)) {
								//状態変更
								//player_->SetBoxState(RectangleFacing::kLandscape);
								isActivate_ = true;
								player_->SetIsChangeRectAnimation(true);
								player_->SetBoxState(RectangleFacing::kLandscape);

								statechange_ = true;


								size_t handle = audio_->SoundLoadWave("Crush.wav");
								size_t crushHandle = audio_->SoundPlayWave(handle);
								audio_->SetValume(crushHandle, 0.1f);

								break;
							}

						}
					}
				}
			}
		}




#pragma endregion

#pragma region ボックスの状態変化
		//
		for (auto& box : boxes_) {
			if (!box->GetIsDead()) {
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
		}
#pragma endregion

#pragma region シーン切り替え関係
		//ボックスがすべてない状態の時
		if (isHitClearBox_) {
			//ステージセレクト時のコリジョン
			if (isStageSelect_) {
				for (auto& stage : selectStage_) {
					//ゴールにあったらシーン転換
					if (stage->IsHitCollision(player_->collider_)) {
						//シーン変更フラグをON
						player_->isGoal_ = true;
						nextmapPass_ = stage->GetFlyingStageNum();
						player_->goalPos_ = stage->GetWorldTransform()->translation_;

						break;
					}
				}
			}
			else {
				//クリアボックスとプレイヤーとの当たり判定&&長方形の向き状態が同じ
				if (clearBox_->IsHitCollision(player_->collider_) && player_->GetRectangle() == clearBox_->GetRectangle()) {
					//シーン変更フラグをON
					player_->isGoal_ = true;
					/*size_t handle = audio_->SoundLoadWave("Clear.wav");
					size_t clearHandle = audio_->SoundPlayWave(handle);
					audio_->SetValume(clearHandle, 0.1f);*/
				}
			}

		}

		//プレイヤーが死んだら初期化
		if (player_->GetIsDead()) {
			//シーン変更フラグをON
			isStageChange_ = true;
			nextmapPass_ = mapPassNum_ - 1;
			//StageInitialize(mapPassNum_ - 1);
		}

#pragma endregion	

	}

}

void GameScene::AllCollisionPrePosUpdate() {
	//ブロックとの押し出し処理
	for (auto& wall : map_->GetWallCollider()) {
		wall->prePosUpdate();
	}
	player_->collider_.prePosUpdate();
	for (auto& box : boxes_) {
		if (box->GetIsDead()) {
			box->GetCollider()->prePosUpdate();
		}
	}

}

void GameScene::CheckBoxDead() {
	//仮で-50以下で消滅するように
	float deadLine = 32.0f;

	//死んだ数チェック
	int alliveNum_ = 0;

	for (auto& box : boxes_) {
		if (box->GetIsDead()) {

		}
		else {
			alliveNum_++;

			Vector3 pos = MakeTranslation(box->GetWorldTransform()->matWorld_);
			if (pos.y <= -deadLine) {
				//死亡判定渡し
				box->SetIsDead(true);
			}
		}
	}

	//すべて死んでいる場合
	if (alliveNum_ == 0) {
		isHitClearBox_ = true;
	}
}

void GameScene::InGameSceneChange() {

	if (isStageChange_) {
		sceneAnime_ = SceneAnimation::kEnd;
		map_->SetAnimeRZ(map_->GetWorldTransform()->rotation_.z);
	}

	if (input_->TriggerKey(DIK_2)) {
		isStageChange_ = true;
	}
}

void GameScene::NextScene() {
	if (isStageChange_) {
		if (nextmapPass_) {
			mapPassNum_ = nextmapPass_.value();
			nextmapPass_ = std::nullopt;
		}

		if (mapPassNum_ < Map::maxMapNum_) {
			StageInitialize(mapPassNum_);
			isStageChange_ = false;
			isHitClearBox_ = false;

			if (mapPassNum_ == 0) {
				isStageSelect_ = true;
			}
			else {
				isStageSelect_ = false;
			}
		}
		else {
			//sceneRequest_ = Scene::Clear;
			mapPassNum_ = 0;
			StageInitialize(mapPassNum_);
			isStageChange_ = false;
			isHitClearBox_ = false;
			isStageSelect_ = true;
		}
	}

	sceneAnime_ = SceneAnimation::kStart;
}

void GameScene::ClearInitialize() {

}

void GameScene::ClearUpdate() {

}

void GameScene::ModelDraw() {
	switch (scene_) {
	case GameScene::Scene::Title:
		break;
	case GameScene::Scene::InGame:
		title_->Draw();
		if (!isStageSelect_) {
			clearBox_->Draw();
		}

		if (!player_->GetIsDead()) {
			player_->Draw();
		}

		for (auto& box : boxes_) {
			box->Draw();
		}

		//セレクトシーンだけ更新
		if (isStageSelect_) {
			for (auto& stagesele : selectStage_) {
				stagesele->Draw();
			}
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
		deadParticle_->Draw(&viewProjection_, { 1.0f,1.0f,1.0f,1.0f }, bikkuri_);
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
	if (isPause_) {
		halfBlack_->Draw();
		gameCloseSprite_->Draw();
		stageSelectSprite_->Draw();
		titleSelectSprite_->Draw();
		if (pauseSelectNum_ == 0) {
			selectSprite_->position_ = { WinApp::kWindowWidth / 2.0f - 226.0f - 30.0f, titleSelectSprite_->position_.y };
		}
		else if (pauseSelectNum_ == 1) {
			selectSprite_->position_ = { WinApp::kWindowWidth / 2.0f - 226.0f - 30.0f, stageSelectSprite_->position_.y };
		}
		else {
			selectSprite_->position_ = { WinApp::kWindowWidth / 2.0f - 226.0f - 30.0f, gameCloseSprite_->position_.y };
		}
		selectSprite_->Draw();

	}
	switch (scene_) {
	case GameScene::Scene::Title:
		if (!isTitleCameraMove) {
			pushASprite_->Draw();
		}
		break;
	case GameScene::Scene::InGame:
		switch (inGameScene)
		{
		case GameScene::Title:
			break;
		case GameScene::InGame:
			map_->DrawSprite();


			if (isStageSelect_ == true) {
				rotateSprite_->Draw();
				moveSprite_->Draw();
				jumpSprite_->Draw();

				for (int i = 0; i < 10; i++) {
					if (selectStage_[i]->GetFlyingStageNum() == 1) {
						oneSprite_->position_ = viewProjection_.MakeScreenVector(MakeTranslation(selectStage_[i]->GetWorldTransform()->matWorld_));
					}
					else if (selectStage_[i]->GetFlyingStageNum() == 2) {
						twoSprite_->position_ = viewProjection_.MakeScreenVector(MakeTranslation(selectStage_[i]->GetWorldTransform()->matWorld_));
					}
					else if (selectStage_[i]->GetFlyingStageNum() == 3) {
						threeSprite_->position_ = viewProjection_.MakeScreenVector(MakeTranslation(selectStage_[i]->GetWorldTransform()->matWorld_));
					}
					else if (selectStage_[i]->GetFlyingStageNum() == 4) {
						fourSprite_->position_ = viewProjection_.MakeScreenVector(MakeTranslation(selectStage_[i]->GetWorldTransform()->matWorld_));
					}
					else if (selectStage_[i]->GetFlyingStageNum() == 5) {
						fiveSprite_->position_ = viewProjection_.MakeScreenVector(MakeTranslation(selectStage_[i]->GetWorldTransform()->matWorld_));
					}
					else if (selectStage_[i]->GetFlyingStageNum() == 6) {
						sixSprite_->position_ = viewProjection_.MakeScreenVector(MakeTranslation(selectStage_[i]->GetWorldTransform()->matWorld_));
					}
					else if (selectStage_[i]->GetFlyingStageNum() == 7) {
						sevenSprite_->position_ = viewProjection_.MakeScreenVector(MakeTranslation(selectStage_[i]->GetWorldTransform()->matWorld_));
					}
					else if (selectStage_[i]->GetFlyingStageNum() == 8) {
						eightSprite_->position_ = viewProjection_.MakeScreenVector(MakeTranslation(selectStage_[i]->GetWorldTransform()->matWorld_));
					}
					else if (selectStage_[i]->GetFlyingStageNum() == 9) {
						nineSprite_->position_ = viewProjection_.MakeScreenVector(MakeTranslation(selectStage_[i]->GetWorldTransform()->matWorld_));
					}
					else if (selectStage_[i]->GetFlyingStageNum() == 10) {
						tenSprite_->position_ = viewProjection_.MakeScreenVector(MakeTranslation(selectStage_[i]->GetWorldTransform()->matWorld_));
					}

				}

				oneSprite_->Draw();
				twoSprite_->Draw();
				threeSprite_->Draw();
				fourSprite_->Draw();
				threeSprite_->Draw();
				fiveSprite_->Draw();
				sixSprite_->Draw();
				sevenSprite_->Draw();
				eightSprite_->Draw();
				nineSprite_->Draw();
				tenSprite_->Draw();
			}
			if (mapPassNum_ == 2) {
				dropSprite_->Draw();
			}
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

void GameScene::PostUIDraw() {
	pauseSprite_->Draw();
	switch (scene_) {
	case GameScene::Scene::Title:
		break;
	case GameScene::Scene::InGame:
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

	// 背景スプライト描画
	Sprite::PreDraw(commandContext);
	PostSpriteDraw();
	Sprite::PostDraw();
}

void GameScene::UIDraw(CommandContext& commandContext) {
	// 前景スプライト描画
	Sprite::PreDraw(commandContext);
	PostUIDraw();
	Sprite::PostDraw();
}
