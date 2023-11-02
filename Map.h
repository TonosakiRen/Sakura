#pragma once


#include "Input.h"
#include "GameObject.h"

#include <optional>
#include <vector>

#include<numbers>

#include "Collider.h"

class Map :
	public GameObject {
public:

	static bool isRotating;

	//
	void Initialize(const std::string name, ViewProjection* viewProjection, DirectionalLight* directionalLight);

	void Update();

	void Draw();

#pragma region ゲッター

	// マップの壁一つ
	std::vector<WorldTransform> GetSpikeWorld() { return worlds_; }

	// マップ中心座標取得
	const Vector3& GetMapCenter() {
		return {
			worldTransform_.matWorld_.m[3][0],
			worldTransform_.matWorld_.m[3][1],
			worldTransform_.matWorld_.m[3][2],
		};
	}

	//
	const WorldTransform& GetPlayerW() { return playerWorld_; }

	const bool& GetIsRotating() { return isRotating; }

	std::vector<Collider> GetCollider() { return colliders_; }
		
#pragma endregion

private://メンバ関数


	//状態リクエスト処理
	void RequestProcessing();

	//状態ごとの更新
	void StateUpdate();


#pragma region 状態初期化関数

	void InitializeStateNormal();

	void InitializeStateRightRotation();

	void InitializeStateLeftRotation();

#pragma endregion

#pragma region 状態更新
	//Normal状態の更新
	void UpdateStateNormal();
	//右回転処理の更新
	void UpdateStateRightRotation();
	//左回転処理の更新
	void UpdateStateLeftRotation();

#pragma endregion



private://変数
	Input* input = nullptr;

	// タイルサイズ
	static const int mapTileNumX_ = 11;
	static const int mapTileNumY_ = 11;

	// 蒲田の四角のサイズ
	const float tileWide_ = 2.0f;

	enum Tile {
		None,
		Block,
		Player,
	};

	const int mapTile_[mapTileNumY_][mapTileNumX_] = {
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 2, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	};

	// マップチップ別のワールド
	std::vector<WorldTransform> worlds_;

	// マップチップ別のコライダー
	std::vector<Collider> colliders_;

	// プレイヤーのワールド
	WorldTransform playerWorld_;


#pragma region 状態
	// マップ状態
	enum class State {
		kNormal,        // 何もない
		kRightRotation, // 右回転
		kLeftRotation   // 左回転
	};

	// 状態
	State state_ = State::kNormal;
	// 状態リクエスト
	std::optional<State> stateRequest_ = std::nullopt;

#pragma endregion


#pragma region 回転処理
	const float HalfPI_ = (float)std::numbers::pi / 2.0f;

	float t_ = 0;

	const float addT_ = 1.0f / 30.0f;

	//回転構造体
	struct RotationEasing {
		float startT;
		float endT;
	};

	RotationEasing rotateE_;

#pragma endregion
};