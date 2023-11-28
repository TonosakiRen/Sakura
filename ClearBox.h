#pragma once
#include "GameObject.h"
#include"Collider.h"
#include"States.h"

class ClearBox :
	public GameObject {

public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="name"></param>
	/// <param name="viewProjection"></param>
	/// <param name="directionalLight"></param>
	/// <param name="pWorld"></param>
	void Initialize(const std::string name, ViewProjection* viewProjection, DirectionalLight* directionalLight, WorldTransform gWorld);

	void StageInitialize(WorldTransform gWorld,int stagenum);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// コリジョンの判定と処理
	/// </summary>
	/// <param name="otherCollider"></param>
	void Collision(Collider& otherCollider);

	/// <summary>
	/// 当たってるか否かの判定のみ処理
	/// </summary>
	/// <param name="otherCollider">ほかのコライダー</param>
	/// <returns></returns>
	bool IsHitCollision(Collider& otherCollider);

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// 向き取得
	/// </summary>
	RectangleFacing GetRectangle() { return rectangleState_; }


private:
	//縦向きスケール
	Vector3 portraitScale = { 0.8f,1.5f,1.0f };
	//横向きスケール
	Vector3 landScapeScale = { 1.5f,0.8f,1.0f };


	Collider collider_;

	//ボックスの状態
	RectangleFacing rectangleState_ = RectangleFacing::kLandscape;
};

