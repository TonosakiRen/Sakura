#pragma once
#include "DirectXCommon.h"
#include "Model.h"
#include "ViewProjection.h"
#include "WorldTransform.h"
#include "Input.h"
#include "Sprite.h"
#include "DirectionalLight.h"
#include "Particle.h"
#include "ParticleBox.h"
#include "GameObject.h"
#include "Player.h"
#include "Map.h"
#include"Box.h"
#include"ClearBox.h"

#include "DeadLineParticle.h"

#include <optional>
class GameScene {

public:
	GameScene();
	~GameScene();

	void Initialize();
	void Update();
	void ModelDraw();
	void ParticleDraw();
	void ParticleBoxDraw();
	void PreSpriteDraw();
	void PostSpriteDraw();
	void Draw(CommandContext& commandContext);
	void UIDraw(CommandContext& commandContext);

private: //メンバ関数

#pragma region タイトルのメンバ変数

	void TitleInitialize();
	void TitleUpdate();
#pragma endregion

#pragma region インゲームのメンバ変数
	//インゲーム
	void InGameInitialize();
	void InGameUpdate();

	void StageInitialize(int stageNum);

	//コリジョン
	void AllCollision();
	void AllCollisionPrePosUpdate();

	//deadCheck
	void CheckBoxDead();

	//InGameでのシーンチェンジの管理
	void InGameSceneChange();

#pragma endregion

#pragma region クリア
	void ClearInitialize();
	void ClearUpdate();
#pragma endregion


private://メンバ関数
	DirectXCommon* dxCommon_ = nullptr;
	Input* input_ = nullptr;

	ViewProjection viewProjection_;
	DirectionalLight directionalLight_;

	uint32_t textureHandle_;

	std::unique_ptr<Sprite>sprite_;
	WorldTransform spriteTransform_;

	std::unique_ptr<GameObject> sphere_;

	std::unique_ptr<Player> player_;

	std::unique_ptr<Map> map_;

	//箱
	std::vector<std::unique_ptr<Box>>boxes_;

	//クリアボックス
	std::unique_ptr<ClearBox>clearBox_;

	std::unique_ptr<DeadLineParticle> deadParticle_;

	//Scene
	enum class Scene {
		Title,
		InGame,
		Clear,
		SceneNum
	};

	Scene scene_ = Scene::InGame;
	Scene nextScene = Scene::InGame;
	static void (GameScene::* SceneInitializeTable[])();
	static void (GameScene::* SceneUpdateTable[])();
	std::optional<Scene> sceneRequest_ = std::nullopt;


	int mapPassNum_ = 0;

	bool isHitClearBox_ = false;

	//SceneChangeするかのフラグ
	bool isStageChange_ = false;
	//Collider* ComebackCollider(Box* baseBox);

	uint32_t bikkuri_;

	const int maxMapNum_ = 10;

	enum class SceneAnimation {
		kStart,
		kInGame,
		kEnd
	};

	SceneAnimation sceneAnime_ = SceneAnimation::kInGame;
};

