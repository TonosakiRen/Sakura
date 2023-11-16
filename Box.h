#pragma once
#include"GameObject.h"
#include"Collider.h"


enum BOX_STATE {
	kFall,
	kStay
};


class Box : public GameObject {

public:

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="name"></param>
	/// <param name="viewProjection"></param>
	/// <param name="directionalLight"></param>
	/// <param name="world">各種設定を行ったworld</param>
	void Initialize(const std::string name, ViewProjection* viewProjection, DirectionalLight* directionalLight,
		const WorldTransform&world,const int managementNum);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	void Collision(Collider& otherCollider);

	bool IsCollision(Collider& otherCollider);

	void CollisionUnderCollider(Collider& other);

	//再起関数内で使用
	bool IsCollisionRecurrence(Collider& other);


	void StateChange();

	void SetCollisionFlagTrue() { isAlreadyCollision_ = true; }

	void SetState(BOX_STATE state) { state_ = state; }

public://ゲッター

	Collider* GetCollider(){ return collider_.get(); }

	const int GetMaagementNum()const { return managementNum_; }

	bool GetIsAlreadCollision(){return isAlreadyCollision_; }
private:


	BOX_STATE state_ = kFall;

	//コライダー
	std::unique_ptr<Collider>collider_;

	//アンダーコライダー
	std::unique_ptr<Collider>underCollider_;

	//管理番号
	int managementNum_;

	//ボックス同士のコリジョン処理をしたかどうか
	bool isAlreadyCollision_ = false;

	//埋まっているか
	bool isBuried_ = false;

	
	//重力
	const Vector3 gravity_ = { 0.0f,-0.5f,0.0f };


};