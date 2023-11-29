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
#include "Box.h"
#include "ClearBox.h"
#include "PlayerAnimation.h"

#include "DeadLineParticle.h"

#include"SelectStage.h"


#include <optional>
class Audio;

class GameScene {

public:

	static bool shutDown;

	GameScene();
	~GameScene();

	void Initialize();
	void Update();
	void ModelDraw();
	void ParticleDraw();
	void ParticleBoxDraw();
	void PreSpriteDraw();
	void PostSpriteDraw();
	void PostUIDraw();
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

	void NextScene();
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
	std::unique_ptr<ClearBox>clearBox_=nullptr;

	//シーンセレクトボックス
	std::vector<std::unique_ptr<SelectStage>>selectStage_;

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

	
	enum class SceneAnimation {
		kStart,
		kInGame,
		kEnd
	};

	//これがホントのシーンってな
	enum InGameScene {
		Title,
		InGame
	};

	InGameScene inGameScene = InGameScene::Title;
	bool isTitleCameraMove = false;
	float titleCameraT = 0.0f;


	SceneAnimation sceneAnime_ = SceneAnimation::kInGame;

	//playerAnimation]
	std::unique_ptr<PlayerAnimation> playerAnimation_;

	static int spawnBoxNum;


	std::unique_ptr<GameObject> title_;

	float titleAnimationYpos_=0;

	float titleAnimeT_ = 0;

	float ChangeY_ = 0;

	Vector3 aa;


	bool isStageSelect_ = false;

	std::optional<int> nextmapPass_ = std::nullopt;
	std::unique_ptr<Sprite> halfBlack_;

	bool isPause_ = false;
	int pauseSelectNum_ = 0;

	bool isBackTitle = false;

	Audio* audio_ = nullptr;

	std::unique_ptr<Sprite> gameCloseSprite_;
	std::unique_ptr<Sprite> stageSelectSprite_;
	std::unique_ptr<Sprite> titleSelectSprite_;
	std::unique_ptr<Sprite> selectSprite_;
	std::unique_ptr<Sprite> pushASprite_;

	

	std::unique_ptr<Sprite> rotateSprite_;
	std::unique_ptr<Sprite> moveSprite_;
	std::unique_ptr<Sprite> jumpSprite_;
	std::unique_ptr<Sprite> dropSprite_;

	std::unique_ptr<Sprite> pauseSprite_;

	std::unique_ptr<Sprite> oneSprite_;
	std::unique_ptr<Sprite> twoSprite_;
	std::unique_ptr<Sprite> threeSprite_;
	std::unique_ptr<Sprite> fourSprite_;
	std::unique_ptr<Sprite> fiveSprite_;
	std::unique_ptr<Sprite> sixSprite_;
	std::unique_ptr<Sprite> sevenSprite_;
	std::unique_ptr<Sprite> eightSprite_;
	std::unique_ptr<Sprite> nineSprite_;
	std::unique_ptr<Sprite> tenSprite_;

	uint32_t maxBoxNum = 0;

	int saveBoxNum = 0;
	float colorT = 0.0f;

};

