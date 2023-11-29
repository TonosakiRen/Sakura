#pragma once
#include "Player.h"
class PlayerAnimation
{
public:

	void Initialize();

	void Update();

	void SetPlayer(Player* player) {
		player_ = player;
		slimeWorldTransform_ = player->GetSlimeWorldTransform();
	}

	Vector3 pos;
	Vector3 prePos;

	float playerHight = 3.0f;

	float softSpeed = 0.002f;
	float softVel = 0.0f;

	int direction = 0;
	int preDirection = 0;

	Player* player_;
	WorldTransform* slimeWorldTransform_;

	WorldTransform sigaWorldTransform_[Player::slimeNum];

	RectangleFacing savePlayerRectangle = RectangleFacing::kPortrait;
};

