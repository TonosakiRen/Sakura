#pragma once
#include "GameObject.h"
#include "Input.h"
#include "Collider.h"
#include"Box.h"
#include"States.h"

#include<vector>
#include <optional>

//状態
enum class PlayerState {
	kNormal,
	kJump
};


class Audio;

class Player :
	public GameObject
{
friend class PlayerAnimation;
public:

	//初期化
	void Initialize(const std::string name, ViewProjection* viewProjection, DirectionalLight* directionalLight,const WorldTransform& pWorld);

	//初期化
	void StageInitialize(const WorldTransform& pWorld);

	//更新
	void Update();

	void UpdateMatiries();

	//描画
	void Draw();

	//コリジョン判定と処理
	void Collision(Collider& otherCollider,const Vector3& priotiyVector);

	//コリジョン判定と処理
	void Collision(Collider& otherCollider);

	//ぴったり配置されているか
	bool IsSetPerfect(Collider& otherCollider);

	//当たったか否か
	bool IsCollision(Collider& otherCollider);

	//下のコライダーのフラグ
	void UnderColliderCollision(Collider& otherCollider);

	//↓コライダーに当たったか否か
	bool IsUnderColliderCollision(Collider& otherCollider);

	//上にあるコライダーに当たったか否か
	bool IsUpColliderCollision(Collider& otherCollider);

	/// <summary>
	/// 向き取得
	/// </summary>
	RectangleFacing GetRectangle() { return rectangleState_; }

	//状態更新
	void SetState(PlayerState state) {
		//
		if (state_ != state) {
			stateRequest_ = state;
		}
	}

	void SetBoxState(RectangleFacing rec) {
		
		if (worldTransform_.scale_.x==portraitScale.x&& worldTransform_.scale_.y == portraitScale.y&& worldTransform_.scale_.z == portraitScale.z) {
			worldTransform_.scale_ = landScapeScale;
		}
		else {
			worldTransform_.scale_ = portraitScale;
		}

		rectangleState_ = rec;

	}

	//初期化なしで状態変更
	void SetStateNoInitialize(PlayerState state) {
		state_ = state;
	}

	//初速度0でジャンプ（落下
	void SetFall() {
		state_ = PlayerState::kJump;
		//velocisity_ = { 0.0f,0.0f,0.0f };
	}


	//引数のシーンと現在のシーンは同じかチェック
	bool CheckStateSame(PlayerState state) {
		if (state_ != state) {
			return false;
		}
		else {
			return true;
		}

	}

	bool CheckBoxStateSame(RectangleFacing rec) {
		if (rectangleState_ == rec) {
			return true;
		}
		
		return false;
	}

	//状態変更の用意があるかチェック
	bool CheckStateReqest() {
		if (stateRequest_) {
			return true;
		}
		return false;
	}

	Vector3 GetMove() {
		return move_;
	}

	void SetIsDead(bool isdead) {
		isDead_ = isdead;
	}

	bool GetIsDead() {
		return isDead_;
	}

	bool GetIsJump() const {
		return isJump;
	}

	void IncrementRotateNum() {
		rotateNum_++;
	}

	void SetIsChangeRectAnimation(bool isChangeRectAnimation) {
		isChangeAnimationRect_ = isChangeRectAnimation;
	}

public:
	//本体のコライダー
	Collider collider_;
	//プレイヤー真下のコライダー
	Collider underCollider_;

	//playerue
	Collider upCollider_;
private:
	//状態更新
	void UpdateState();


private:
	//速度
	Vector3 velocisity_;
	//加速量
	Vector3 acceleration_;
	//移動
	Vector3 move_;
	//キー
	Input* input_;

	//移動速度
	const float spd_ = 0.12f;

public:
	//縦向きスケール
	const Vector3 portraitScale = { 0.8f,1.5f,1.0f };
	//横向きスケール
	const Vector3 landScapeScale = { 1.5f,0.8f,1.0f };

private:

	Audio* audio_ = nullptr;


	//状態変数
	PlayerState state_ = PlayerState::kNormal;
	//状態変更時受け取る
	std::optional<PlayerState> stateRequest_ = std::nullopt;

	
	//埋まって処理を行ったか否か
	bool isBuried_ = false;

	//長方形の向き
	RectangleFacing rectangleState_ = RectangleFacing::kPortrait;

	WorldTransform animationTransform_;

	Model slimeVerticalModel_;
	Model slimeWideModel_;

	public:
	static const uint32_t slimeNum = 10;

	WorldTransform* GetSlimeWorldTransform() {
		return slimeTransform;
	}
	private:

	WorldTransform slimeTransform[slimeNum];

	//死んだか
	bool isDead_ = false;

	bool isJump = false;


 public:

	 bool isChangeAnimationRect_ = false;

	 float changeRectT = 0.0f;

	 //南海開店したか
	 uint32_t rotateNum_ = 0;
	 //ゴールの位置
	 Vector3 goalPos_;

	 bool isGoal_ = false;
	 bool isFinishGoalAnimation_ = false;
	 float goalT_ = 0.0f;

	 bool isChangeScaled = false;

	 bool isMoveSound_ = false;
};