#pragma once
#include "Player.h"
class PlayerAnimation
{
public:

	
	void Update();

	void SetPlayer(Player* player) {
		player_ = player;
		slimeWorldTrasnform_ = player->GetSlimeWorldTransform();
	}

	Player* player_;
	WorldTransform* slimeWorldTrasnform_;

	RectangleFacing savePlayerRectAngele = RectangleFacing::kPortrait;
};

