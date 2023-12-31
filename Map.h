#pragma once


#include "Input.h"
#include "GameObject.h"

#include <optional>
#include <vector>
#include<numbers>
#include<list>

#include"Sprite.h"

#include "Collider.h"

#include "ParticleBox.h"

class Player;

class Map :
	public GameObject {
public:

	static bool isRotating;

	static bool preIsRotating;

	static bool rotateComplete;

	static const uint32_t kBoxNum = 1000;

	Map();

	//
	void Initialize(const std::string name, ViewProjection* viewProjection, DirectionalLight* directionalLight, int num);

	void StageInitialize(int num);

	void Update();

	void UpdateMatrix();

	void Draw();

	void DrawSprite();

	void Finalize();

	void SetAnimeRZ(float z) { zrotate.z = z; }


	void MapEditor(const ViewProjection& view);

	bool StartAnimation();

	bool EndAnimation();

	int GetMaxTile()const { return tileMax_; }
#pragma region ゲッター

	// マップの壁
	std::vector<std::unique_ptr<WorldTransform>>& GetSpikeWorld() { return WallWorlds_; }



	// マップ中心座標取得
	const Vector3 GetMapCenter() {
		return {
			worldTransform_.matWorld_.m[3][0],
			worldTransform_.matWorld_.m[3][1],
			worldTransform_.matWorld_.m[3][2],
		};
	}

	//プレイヤーのWorldTransform
	const WorldTransform& GetPlayerW() { return playerWorld_; }
	//クリアのワールド取得
	const WorldTransform& GetClearW() { return goalW_; }


	//すべてのコライダー取得
	std::vector<std::unique_ptr<Collider>>& GetCollider() { return colliders_; }

	//wallだけのコライダーを取得
	const std::vector<Collider*> GetWallCollider();

	//箱のワールド取得
	std::vector<std::unique_ptr<WorldTransform>>& GetBoxWorldTransform() { return boxWorlds_; }

	//ステージセレクトボックス
	std::vector<std::unique_ptr<WorldTransform>>& GetStageSelectBox() { return stageSelctWorlds_; }

	//マップデータ格納場所
	std::vector<std::vector<int>>& GetMapData_() { return mapData_; };

	void SetPlayer(Player* player) {
		player_ = player;
	}
	
#pragma endregion

private://メンバ関数


	//状態リクエスト処理
	void RequestProcessing();

	//状態ごとの更新
	void StateUpdate();

	//ImGuiまとめ
	void ImGuiDraw();

	//マップデータを基にチップ配置データ初期化
	void MapPositioningInitialize();

#pragma region 状態初期化関数
	//Normal状態の初期化
	void InitializeStateNormal();
	//右回転処理初期化
	void InitializeStateRightRotation();
	//左回転処理初期化
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
	Input* input_ = nullptr;

	Player* player_ = nullptr;

	
	const int tileMax_=5;

#pragma region マップの配置関係
	// タイルサイズ(可変
	int mapTileNumX_ = 1;
	int mapTileNumY_ = 1;

	// 四角の一辺のサイズ
	const float tileWide_ = 2.0f;

public:
	//タイルの種類
	enum Tile {
		None,	//空気
		Block,	//ブロック
		Player,	//プレイヤー初期座標
		Box,	//動く箱
		Goal,	//ゴール
		StageSelectBox //ステージセレクトの箱
	};

	static const int maxMapNum_ = 11;


	static bool isRotationInput_;

private:

	int mapPassNumber_;

	
	//マップのデータ情報格納場所
	const char* map1Pass[maxMapNum_] = {
		"Resources/mapChips/STSelect.txt",
		"Resources/mapChips/Stage1.txt",
		"Resources/mapChips/Stage2.txt",
		"Resources/mapChips/Stage3.txt",
		"Resources/mapChips/Stage4.txt",
		"Resources/mapChips/Stage5.txt",
		"Resources/mapChips/Stage6.txt",
		"Resources/mapChips/Stage7.txt",
		"Resources/mapChips/Stage8.txt",
		"Resources/mapChips/Stage9.txt",
		"Resources/mapChips/Stage10.txt",
	};

	//マップデータ格納場所
	std::vector<std::vector<int>> mapData_;

	std::vector< std::vector<std::vector<int>>>allMapData_;

	// マップチップごとのワールド
	std::vector<std::unique_ptr<WorldTransform>> WallWorlds_;

	// マップチップ別のコライダー
	std::vector<std::unique_ptr<Collider>> colliders_;

	//ステージセレクトのボックス
	std::vector<std::unique_ptr<WorldTransform>>stageSelctWorlds_;

#pragma region 一時的な情報共
	// プレイヤーのワールド
	WorldTransform playerWorld_;

	///Box（移動箱）のWorldの集合体
	std::vector<std::unique_ptr<WorldTransform>>boxWorlds_;

	//ゴールの位置
	WorldTransform goalW_;
#pragma endregion


	

	
#pragma endregion

	


#pragma region 状態
	// マップ状態
	enum class State {
		kNormal,        // 何もない
		kRightRotation, // 右回転
		kLeftRotation   // 左回転
	};

private:

	// 状態
	State state_ = State::kNormal;
	// 状態リクエスト
	std::optional<State> stateRequest_ = std::nullopt;

#pragma endregion


#pragma region 回転処理
	//１/２Π
	const float HalfPI_ = (float)std::numbers::pi / 2.0f;

	//イージングT
	float t_ = 0;

	//１Fの加算量
	const float addT_ = 1.0f / 30.0f;

	//回転構造体
	struct RotationEasing {
		float startT;
		float endT;
	};

	//回転処理に使う
	RotationEasing rotateE_;

	const int maxRotateCooltime_ = 2;

	int rotateCooltime_ = 0;
#pragma endregion

#pragma region 色
	
	Vector4 blockColor = { 1.0f,1.0f,1.0f,1.0f };

	Vector4 blockNormalColor = { 0.0f,0.9f,0.0f,1.0f };
	Vector4 blockRotateColor = { 0.0f,1.0f,0.0f,1.0f };

#pragma endregion


#pragma region Editor
	bool isEditOn_ = false;

	//初期配置処理をしたか
	bool isInitializeEditMode_ = false;

	//編集時のカーソル
	std::unique_ptr<Sprite>editCursor_;

	//カーソルの画像
	uint32_t cursorTex_;

	//レティクルの座標
	Vector2 reticlePos_;

	//参照してるマップ配列のx
	int32_t referenceMapX_ = 0;

	//参照してるマップ配列のy
	int32_t referenceMapY_ = 0;
// _DEBUG

	//Instancingびょうがよう
	std::unique_ptr<ParticleBox> particleBox_;
	

#pragma endregion


#pragma region 初期化用
	WorldTransform iniData_;

	float halfpi = (float)std::numbers::pi / 2.0f;
	//回転量
	const Vector2 stRotatenum = { halfpi * 4 * 2,0 };

	const Vector2 edRotatenum = { 0, -halfpi * 4 * 2 };

	Vector2 zrotate = { 0,0 };

	const float maxAnimeCount_ = 30;
	int animecount_ = 0;
#pragma endregion

};