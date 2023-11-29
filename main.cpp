#include <Windows.h>
#include "WinApp.h"
#include "DirectXCommon.h"
#include "TextureManager.h"
#include "GameScene.h"
#include "ImGuiManager.h"
#include "Particle.h"
#include "ParticleBox.h"
#include "PostEffect.h"
#include "GaussianBlur.h"
// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	WinApp* win = nullptr;
	DirectXCommon* dxCommon = nullptr;
	//汎用機能
	GameScene* gameScene = nullptr;
	Input* input = nullptr;
	ImGuiManager* imguiManager;

	// ゲームウィンドウの作成
	win = WinApp::GetInstance();
	win->CreateGameWindow();
	// DirectX初期化処理
	dxCommon = DirectXCommon::GetInstance();
	dxCommon->Initialize(WinApp::kWindowWidth,WinApp::kWindowHeight);
	// 汎用機能
#pragma region 汎用機能初期化
	input = Input::GetInstance();
	input->Initialize(win->GetHInstance(), win->GetHwnd());

	imguiManager = ImGuiManager::GetInstance();
	imguiManager->Initialize(win);
	// テクスチャマネージャの初期化
	TextureManager::GetInstance()->Initialize();
	TextureManager::Load("white1x1.png");

	// 3Dオブジェクト静的初期化
	Model::StaticInitialize();

	// 3Dオブジェクト静的初期化
	Particle::StaticInitialize();
	ParticleBox::StaticInitialize();

	PostEffect::StaticInitialize();
	GaussianBlur::StaticInitialize();
	dxCommon->InitializePostEffect();

	//　スプライト静的初期化
	Sprite::StaticInitialize();


#pragma endregion 変数

	// ゲームシーンの初期化
	gameScene = new GameScene();
	gameScene->Initialize();

	// メインループ
	while (true) {
		// メッセージ処理
		if (win->ProcessMessage()) {
			break;
		}

		// ImGui受付開始
		imguiManager->Begin();

		// 入力関連の毎フレーム処理
		input->Update();

		// ゲームシーンの毎フレーム処理
		gameScene->Update();
		imguiManager->End();


		// 描画開始
		dxCommon->MainPreDraw();
		// ゲームシーンの描画
		gameScene->Draw(*dxCommon->GetCommandContext());

		dxCommon->MainPostDraw();

		dxCommon->SwapChainPreDraw();

		gameScene->UIDraw(*dxCommon->GetCommandContext());

		// ImGui描画
		imguiManager->Draw();

		dxCommon->SwapChainPostDraw();

		if (GameScene::shutDown == true) {
			break;
		}

	}


	// ImGui解放
	imguiManager->Finalize();
	// ゲームウィンドウの破棄
	win->TerminateGameWindow();

	return 0;
}