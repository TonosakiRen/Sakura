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
	RectangleFacing GetRectangle() { return state_; }
private:

	Collider collider_;


	RectangleFacing state_ = RectangleFacing::kPortrait;
};

