#include "PlayerAnimation.h"
#include "ImGuiManager.h"

void PlayerAnimation::Update()
{
	//切り替えられたとき位置を初期化する
	if (savePlayerRectangle != player_->GetRectangle()) {
		for (int i = 0; i < Player::slimeNum; i++) {
			slimeWorldTransform_[i].translation_ = { 0.0f,0.0f,0.0f };
			slimeWorldTransform_[i].rotation_ = { 0.0f,0.0f,0.0f };
			slimeWorldTransform_[i].scale_ = { 1.0f,1.0f,1.0f };
		}

		if (savePlayerRectangle == RectangleFacing::kPortrait) {

			for (int i = 0; i < Player::slimeNum; i++) {
				slimeWorldTransform_[i].translation_.x = 0.8f - (1.6f / Player::slimeNum) * i - ((1.6f / Player::slimeNum) / 2.0f);
				slimeWorldTransform_[i].rotation_.z = Radian(90.0f);
			}

			savePlayerRectangle = player_->GetRectangle();
			
		}
		else {

			for (int i = 0; i < Player::slimeNum; i++) {
				slimeWorldTransform_[i].translation_.y = 1.5f - (3.0f / Player::slimeNum) * i - ((3.0f / Player::slimeNum) / 2.0f);
			}

			savePlayerRectangle = player_->GetRectangle();

		}
	}
	ImGui::DragFloat3("pl", &slimeWorldTransform_[0].translation_.x,0.01f);
	ImGui::DragFloat3("pr", &slimeWorldTransform_[0].rotation_.x, 0.01f);


	switch (player_->GetRectangle())
	{
	case RectangleFacing::kPortrait: //縦

		pos = player_->GetWorldTransform()->translation_;;

		for (int i = 0; i < Player::slimeNum; i++) {
			

			if (prePos.x != pos.x) {
				if (softMax > soft) {
					soft += softSpeed;
				}

				if (soft > softMax) {
					soft = softMax;
				}
			}
			else {
				if (softMax < 0) {
					soft -= softSpeed * 2.0f;
				}

				if (soft < 0) {
					soft = 0;
				}
			}

			Vector3 slimePos = { 0.0f,0.0f,0.0f };

			//下から生やす - 高さ調整
			slimePos.y =  (i  * ( playerHight / Player::slimeNum)) - ((playerHight - (playerHight / Player::slimeNum)) * 0.5f);

			float distance = pos.x - prePos.x ;

			float t = ((float)i / (float)Player::slimeNum);

			slimePos.x = -distance * t * t  * soft * (float)Player::slimeNum;
			

			slimeWorldTransform_[i].translation_ = slimePos;
		}

		prePos = pos;

		break;
	case RectangleFacing::kLandscape: //横 x が縦　y が　横

		break;
	default:
		break;
	}
	
}
