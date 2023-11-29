#pragma once
#include"GameObject.h"
#include"Collider.h"

class SelectStage:public GameObject {

public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="name"></param>
	/// <param name="viewProjection"></param>
	/// <param name="directionalLight"></param>
	/// <param name="pWorld"></param>
	/// <param name="FlyingStageNumber"></param>
	void Initialize(const std::string name, ViewProjection* viewProjection, DirectionalLight* directionalLight,
		const WorldTransform& pWorld,int FlyingStageNumber);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw(const Vector4& color);

	/// <summary>
	/// 当たってるか否かの判定のみ処理
	/// </summary>
	/// <param name="otherCollider">ほかのコライダー</param>
	/// <returns></returns>
	bool IsHitCollision(Collider& otherCollider);

	
	//飛ぶステージ番号取得
	const int GetFlyingStageNum()const { return flyingStageNum_; }


private:
	//飛ぶステージ番号
	int flyingStageNum_;


	Collider collider_;
};

