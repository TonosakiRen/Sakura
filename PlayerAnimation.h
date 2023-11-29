#pragma once
#include "Player.h"
class PlayerAnimation
{
public:

	
	void Update();

	void SetPlayer(Player* player) {
		player_ = player;
		slimeWorldTransform_ = player->GetSlimeWorldTransform();
	}

	Vector3 pos;
	Vector3 prePos;

	float playerHight = 3.0f;

	float softMax = 1.0f;
	float softSpeed = 0.01f;
	float soft= 0.0f;

	Player* player_;
	WorldTransform* slimeWorldTransform_;

	RectangleFacing savePlayerRectangle = RectangleFacing::kPortrait;
};

