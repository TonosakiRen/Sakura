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
#include "Skydome.h"
#include "Floor.h"
#include "Player.h"
#include "Map.h"
#include"Box.h"

#include <optional>
class GameScene
{

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

private: 
	DirectXCommon* dxCommon_ = nullptr;
	Input* input_ = nullptr;
	
	ViewProjection viewProjection_;
	DirectionalLight directionalLight_;

	uint32_t textureHandle_;

	std::unique_ptr<Sprite>sprite_;
	WorldTransform spriteTransform_;

	std::unique_ptr<Skydome> skydome_;
	
	std::unique_ptr<GameObject> sphere_;

	std::unique_ptr<Player> player_;

	std::unique_ptr<Map> map_;

	//箱
	std::vector<std::unique_ptr<Box>>boxes_;

	//Scene
	enum class Scene {
		Title,
		InGame,

		SceneNum
	};

	Scene scene_ = Scene::InGame;
	Scene nextScene = Scene::InGame;
	static void (GameScene::* SceneInitializeTable[])();
	static void (GameScene::* SceneUpdateTable[])();
	std::optional<Scene> sceneRequest_ = std::nullopt;

	//タイトル
	void TitleInitialize();
	void TitleUpdate();
	//インゲーム
	void InGameInitialize();
	void InGameUpdate();


	//コリジョン
	void AllCollision();

	//Collider* ComebackCollider(Box* baseBox);
};

