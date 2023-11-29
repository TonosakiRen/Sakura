#include "PlayerAnimation.h"
#include "ImGuiManager.h"
#include "Map.h"

void PlayerAnimation::Initialize()
{
	for (int i = 0; i < Player::slimeNum; i++) {
		sigaWorldTransform_[i].translation_ = { 0.0f,0.0f,0.0f };
		sigaWorldTransform_[i].rotation_ = player_->GetWorldTransform()->GetParent()->rotation_;
		sigaWorldTransform_[i].scale_ = { 1.0f,1.0f,1.0f };
	}

	if (savePlayerRectangle == RectangleFacing::kPortrait) {

		for (int i = 0; i < Player::slimeNum; i++) {
			sigaWorldTransform_[i].translation_.y = 0.8f - (1.6f / Player::slimeNum) * i - ((1.6f / Player::slimeNum) / 2.0f);
		}

		savePlayerRectangle = player_->GetRectangle();

	}
	else {

		for (int i = 0; i < Player::slimeNum; i++) {
			sigaWorldTransform_[i].translation_.y = 1.5f - (3.0f / Player::slimeNum) * i - ((3.0f / Player::slimeNum) / 2.0f);
		}

		savePlayerRectangle = player_->GetRectangle();

	}
	
}

void PlayerAnimation::Update()
{

	//切り替えられたとき位置を初期化する
	if (savePlayerRectangle != player_->GetRectangle()) {
		for (int i = 0; i < Player::slimeNum; i++) {
			sigaWorldTransform_[i].translation_ = { 0.0f,0.0f,0.0f };
			sigaWorldTransform_[i].rotation_ = player_->GetWorldTransform()->GetParent()->rotation_;
			sigaWorldTransform_[i].scale_ = { 1.0f,1.0f,1.0f };
		}

		if (savePlayerRectangle == RectangleFacing::kPortrait) {

			for (int i = 0; i < Player::slimeNum; i++) {
				sigaWorldTransform_[i].translation_.y = 0.8f - (1.6f / Player::slimeNum) * i - ((1.6f / Player::slimeNum) / 2.0f);
			}

			savePlayerRectangle = player_->GetRectangle();
			
		}
		else {

			for (int i = 0; i < Player::slimeNum; i++) {
				sigaWorldTransform_[i].translation_.y = 1.5f - (3.0f / Player::slimeNum) * i - ((3.0f / Player::slimeNum) / 2.0f);
			}

			savePlayerRectangle = player_->GetRectangle();

		}
	}
	ImGui::DragFloat3("pl", &sigaWorldTransform_[0].translation_.x,0.01f);
	ImGui::DragFloat3("pr", &sigaWorldTransform_[0].rotation_.x, 0.01f);

	pos = MakeTranslation(player_->GetWorldTransform()->matWorld_);

	if (!Map::isRotating || !player_->isChangeAnimationRect_) {
		switch (player_->GetRectangle())
		{
		case RectangleFacing::kPortrait: //縦


			for (int i = 0; i < Player::slimeNum; i++) {

				if (prePos.x > pos.x) {
					direction = 1;
				}
				else if (prePos.x < pos.x) {
					direction = 0;
				}

			

				Vector3 slimePos = { 0.0f,0.0f,0.0f };

				//下から生やす - 高さ調整
				slimePos.y = (i * (playerHight / Player::slimeNum)) - ((playerHight - (playerHight / Player::slimeNum)) * 0.5f);

				float distance = pos.x - prePos.x;
				
				if (softVel > -distance) {
					softVel -= softSpeed;
				}
				else if (softVel < -distance) {
					softVel += softSpeed;
				}

				float t = ((float)i / (float)Player::slimeNum);

				slimePos.x = t * t  * softVel * (float)Player::slimeNum;
				slimePos.y -= t * t * fabsf(softVel) * 5.0f;

				sigaWorldTransform_[i].translation_ = slimePos;
			}


			break;
		case RectangleFacing::kLandscape: //横


			break;
		default:
			break;
		}
	}
	

	prePos = pos;



	uint32_t index = player_->rotateNum_ % 4;
	Matrix4x4 RotateZMatrix = MakeIdentity4x4();
	RotateZMatrix = MakeRotateZMatrix(Radian(90.0f) * index);

	//xy変換
	for (int i = 0; i < Player::slimeNum;i++) {
		slimeWorldTransform_[i].translation_ = sigaWorldTransform_[i].translation_ * RotateZMatrix;
		slimeWorldTransform_[i].rotation_ = sigaWorldTransform_[i].rotation_;
		slimeWorldTransform_[i].scale_ = sigaWorldTransform_[i].scale_;
	}
}
