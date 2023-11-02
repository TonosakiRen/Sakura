#pragma once

#include "WinApp.h"
#include "externals/imgui/imgui.h"
class ImGuiManager
{
public:
	static ImGuiManager* GetInstance();
	void Initialize(WinApp* winApp);
	void Finalize();
	void Begin();
	void End();
	void Draw();

private:
	ImGuiManager() = default;
	~ImGuiManager() = default;
	ImGuiManager(const ImGuiManager&) = delete;
	const ImGuiManager& operator=(const ImGuiManager&) = delete;
};

