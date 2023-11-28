#pragma once

#include"Player.h"
#include"Map.h"
#include"Box.h"
#include"ClearBox.h"


class StageScene {

public:

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="mapPassNum">マップパスへの番号</param>
	/// <param name="viewProjection"></param>
	/// <param name="directionalLight"></param>
	void Initialize(nt mapPassNum,ViewProjection* viewProjection, DirectionalLight* directionalLight);

	/// <summary>
	/// 更新
	/// </summary>
	/// <param name="viewProjection"></param>
	void Update(ViewProjection* viewProjection);

	/// <summary>
	/// モデル描画
	/// </summary>
	void ModelDraw();

	/// <summary>
	/// パーティクル描画
	/// </summary>
	void ParticleDraw();

	/// <summary>
	/// 背景スプライト
	/// </summary>
	void PreSpriteDraw();

	/// <summary>
	/// 前景スプライト
	/// </summary>
	void PostSpriteDraw();
	

private:
	//コリジョン
	void AllCollision();

	//deadCheck
	void CheckBoxDead();

	//InGameでのシーンチェンジの管理
	void InGameSceneChange();

private:
	Input* input_ = nullptr;

	std::unique_ptr<Player> player_;

	std::unique_ptr<Map> map_;

	//箱
	std::vector<std::unique_ptr<Box>>boxes_;

	//クリアボックス
	std::unique_ptr<ClearBox>clearBox_;

	//マップのデータ
	int mapPassNum_ = 0;

	bool isHitClearBox_ = false;

	//SceneChangeするかのフラグ
	bool isSceneChange_ = false;
};

