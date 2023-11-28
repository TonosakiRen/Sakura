//#include "StageScene.h"
//
//void StageScene::Initialize(int mapPassNum, ViewProjection* viewProjection, DirectionalLight* directionalLight) {
//	mapPassNum_ = mapPassNum;
//	
//	map_->Initialize("stage", viewProjection, directionalLight, mapPassNum_);
//
//	player_->Initialize("player", viewProjection, directionalLight, map_->GetPlayerW());
//
//	//箱の初期化
//	int managementNum = 0;
//	for (auto& world : map_->GetBoxWorldTransform()) {
//		std::unique_ptr<Box>box = std::make_unique<Box>();
//		box->Initialize("box", viewProjection, directionalLight, *world.get(), managementNum);
//
//		boxes_.emplace_back(std::move(box));
//
//		managementNum++;
//	}
//
//	clearBox_->Initialize("player", viewProjection, directionalLight, map_->GetClearW());
//
//}
//
//void StageScene::Update(ViewProjection* viewProjection) {
//	/*
//	if (input_->TriggerKey(DIK_P)) {
//		sceneRequest_ = Scene::Title;
//	}
//	*/
//
//	map_->Update();
//	map_->MapEditor(*viewProjection);
//
//	clearBox_->Update();
//
//	player_->Update();
//
//	for (auto& box : boxes_) {
//		box->Update();
//	}
//
//	//マップが回転していないときのみコリジョン処理
//	if (!Map::isRotating) {
//		AllCollision();
//	}
//
//	//ボックスの枠外落下処理
//	CheckBoxDead();
//
//	//条件を満たしている場合のシーンチェンジ
//	InGameSceneChange();
//}
//
//void StageScene::ModelDraw() {
//	map_->Draw();
//
//	clearBox_->Draw();
//	player_->Draw();
//	for (auto& box : boxes_) {
//		box->Draw();
//	}
//}
//
//void StageScene::ParticleDraw() {
//
//}
//
//
//void StageScene::PreSpriteDraw() {
//}
//
//void StageScene::PostSpriteDraw() {
//	map_->DrawSprite();
//}
//
//void StageScene::AllCollision() {
//
//#pragma region 押し戻し処理
//	//ブロックとの押し出し処理
//	for (auto& wall : map_->GetWallCollider()) {
//		player_->Collision(*wall);
//
//
//		for (auto& box : boxes_) {
//			if (!box->GetIsDead()) {
//				box->Collision(*wall);
//			}
//		}
//	}
//
//
//	//ボックス処理
//	for (auto& box : boxes_) {
//		if (!box->GetIsDead()) {
//			//先にプレイヤーにあわせて押し出し
//			box->Collision(player_->collider_);
//
//
//			//二個目のボックス処理
//			for (auto& box2 : boxes_) {
//				//一個目をもとに押し戻し処理
//				if (box2->GetMaagementNum() != box->GetMaagementNum()) {
//					box2->Collision(*box->GetCollider());
//
//
//					//押し出しによって壁に埋まったら押し出す
//					for (auto& wall : map_->GetWallCollider()) {
//						box2->Collision(*wall);
//					}
//
//
//					for (auto& box3 : boxes_) {
//						if (box3->GetMaagementNum() != box2->GetMaagementNum()) {
//							box3->Collision(*box2->GetCollider());
//
//							//押し出しによって壁に埋まったら押し出す
//							for (auto& wall : map_->GetWallCollider()) {
//								box3->Collision(*wall);
//							}
//
//							//押し出された箱２に押し戻された処理
//							box2->Collision(*box3->GetCollider());
//						}
//					}
//
//
//					//押し出された箱２に押し戻された処理
//					box->Collision(*box2->GetCollider());
//				}
//			}
//
//
//
//			//壁チップ検索
//			for (auto& wall : map_->GetWallCollider()) {
//				//プレイヤーで埋まっていたまたは元々埋まっていた場合押し出し処理
//				box->Collision(*wall);
//			}
//
//
//			//押し戻しによって返された分プレイヤーも返す
//			player_->Collision(*box->GetCollider());
//		}
//	}
//
//#pragma endregion
//
//#pragma region プレイヤーの状態変化
//	bool isActivate_ = false;
//
//	//壁との処理
//	for (auto& wall : map_->GetWallCollider()) {
//		//下のコライダーとの当たり判定処理
//		if (player_->IsUnderColliderCollision(*wall)) {
//
//			//ぴったりくっついている場合
//			if (player_->IsSetPerfect(*wall)) {
//
//				//状態変更（ノーマル
//				player_->SetState(PlayerState::kNormal);
//				isActivate_ = true;
//				break;
//			}
//
//		}
//	}
//
//	//ボックスとの処理
//	for (auto& box : boxes_) {
//		if (!box->GetIsDead()) {
//			//下のコライダーとの当たり判定処理
//			if (player_->IsUnderColliderCollision(*box->GetCollider())) {
//
//				//ぴったりくっついている場合
//				if (player_->IsSetPerfect(*box->GetCollider())) {
//
//					//状態変更（ノーマル
//					player_->SetState(PlayerState::kNormal);
//					isActivate_ = true;
//					break;
//				}
//			}
//
//			if (!player_->CheckStateReqest()) {
//				//Boxが縦向きの時の処理
//				if (player_->CheckBoxStateSame(RectangleFacing::kPortrait)) {
//					//上のコライダーとの当たり判定
//					if (player_->IsUpColliderCollision(*box->GetCollider())) {
//						//ぴったりくっついている場合&&地面にいる場合
//						if (player_->IsSetPerfect(*box->GetCollider()) && player_->CheckStateSame(PlayerState::kNormal)) {
//
//
//							//状態変更
//							player_->SetBoxState(RectangleFacing::kLandscape);
//							isActivate_ = true;
//							break;
//						}
//					}
//				}
//			}
//		}
//	}
//
//
//	if (!isActivate_) {
//		player_->SetStateNoInitialize(PlayerState::kJump);
//	}
//#pragma endregion
//
//#pragma region ボックスの状態変化
//	//
//	for (auto& box : boxes_) {
//
//		bool isBoxHit = false;
//		for (auto& wall : map_->GetWallCollider()) {
//			//ボックスの下にあるコライダーが浮いているか否か
//			if (box->CollisionUnderCollider(*wall)) {
//
//				if (!isBoxHit) {
//					if (box->IsSetPerfect(*wall)) {
//						//状態を変更
//						box->SetState(kStay);
//						isBoxHit = true;
//					}
//				}
//			}
//		}
//
//		for (auto& box2 : boxes_) {
//			if (!box2->GetIsDead()) {
//				//ボックスの下にあるコライダーが浮いているか否か
//				if (box->CollisionUnderCollider(*box2->GetCollider())) {
//
//					if (!isBoxHit) {
//						if (box->IsSetPerfect(*box2->GetCollider())) {
//							//状態を変更
//							box->SetState(kStay);
//							isBoxHit = true;
//						}
//					}
//				}
//			}
//		}
//
//		//当たっていなかったので変更
//		if (!isBoxHit) {
//			box->SetState(kFall);
//		}
//	}
//#pragma endregion
//
//
//
//	//ボックスがすべてない状態の時
//	if (isHitClearBox_) {
//		//クリアボックスとプレイヤーとの当たり判定&&長方形の向き状態が同じ
//		if (clearBox_->IsHitCollision(player_->collider_) && player_->GetRectangle() == clearBox_->GetRectangle()) {
//			//シーン変更フラグをON
//			isSceneChange_ = true;
//		}
//	}
//}
//
//void StageScene::CheckBoxDead() {
//	//仮で-50以下で消滅するように
//	float deadLine = -50;
//
//	//死んだ数チェック
//	int deadNum_ = 0;
//
//	for (auto& box : boxes_) {
//		if (box->GetIsDead()) {
//			deadNum_++;
//		}
//		else if (box->GetWorldTransform()->matWorld_.m[3][1] <= deadLine) {
//			//死亡判定渡し
//			box->SetIsDead(true);
//		}
//
//
//	}
//
//	//すべて死んでいる場合
//	if (deadNum_ == boxes_.size()) {
//		isHitClearBox_ = true;
//	}
//}
//
//void StageScene::InGameSceneChange() {
//	if (isSceneChange_) {
//		//シーンを変更
//		
//	}
//}
