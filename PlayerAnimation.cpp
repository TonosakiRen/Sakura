#include "PlayerAnimation.h"
#include "ImGuiManager.h"

void PlayerAnimation::Update()
{
	//切り替えられたとき位置を初期化する
	if (savePlayerRectAngele != player_->GetRectangle()) {
		if (savePlayerRectAngele == RectangleFacing::kPortrait) {

			for (int i = 0; i < Player::slimeNum; i++) {
				slimeWorldTrasnform_[i].translation_.x = 0.8f - (1.6f / Player::slimeNum) * i - ((1.6f / Player::slimeNum) / 2.0f);
				slimeWorldTrasnform_[i].translation_.y = 0.0f;
				slimeWorldTrasnform_[i].rotation_.z = Radian(90.0f);
			}

			savePlayerRectAngele = player_->GetRectangle();
			
		}
		else {

			for (int i = 0; i < Player::slimeNum; i++) {
				slimeWorldTrasnform_[i].translation_.y = 1.5f - (3.0f / Player::slimeNum) * i - ((3.0f / Player::slimeNum) / 2.0f);
				slimeWorldTrasnform_[i].translation_.x = 0.0f;
				slimeWorldTrasnform_[i].rotation_.z = 0.0f;
			}

			savePlayerRectAngele = player_->GetRectangle();

		}
	}
	ImGui::DragFloat3("pl", &slimeWorldTrasnform_[0].translation_.x,0.01f);
	ImGui::DragFloat3("pr", &slimeWorldTrasnform_[0].rotation_.x, 0.01f);

	switch (player_->GetRectangle())
	{
	case RectangleFacing::kPortrait: //縦

		break;
	case RectangleFacing::kLandscape: //横 x が縦　y が　横

		break;
	default:
		break;
	}
	
}
