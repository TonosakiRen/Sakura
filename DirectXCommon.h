#pragma once
#include <Windows.h>
#include <cstdlib>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <vector>
#include <dxcapi.h>
#pragma comment(lib,"dxcompiler.lib")
#include "externals/DirectXTex/d3dx12.h"
#include "WinApp.h"
#include "DescriptorHeap.h"
#include "CommandQueue.h"
#include "SwapChain.h"
#include "ColorBuffer.h"
#include "DepthBuffer.h"
#include "PostEffect.h"
#include "CommandContext.h"
#include "GaussianBlur.h"
#include "Bloom.h"

class DirectXCommon
{
public:

	//うんち設計
	uint32_t kSrvHeapDescritorNum = 1024;
	uint32_t kRtvHeapDescritorNum = 32;

	//mainColorBufferNum
	uint32_t kMainColorBufferNum = 1;

	static DirectXCommon* GetInstance();
	void Initialize(int32_t backBufferWidth = 1280, int32_t backBufferHeight = 720);
	void InitializePostEffect();

	void MainPreDraw();
	void MainPostDraw();
	void SwapChainPreDraw();
	void SwapChainPostDraw();

	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(const std::wstring& filePath, const wchar_t* profile);

	DescriptorHandle AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type);

	ID3D12Device* GetDevice() { return device_.Get(); }
	CommandQueue& GetCommandQueue() { return commandQueue_; }
	CommandContext* GetCommandContext() { return &commandContext_; }

	DescriptorHeap& GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type) { return descriptorHeaps_[type]; }

	void ClearMainDepthBuffer() { commandContext_.ClearDepth(mainDepthBuffer_); }

private:
	WinApp* winApp_;

	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_;
	Microsoft::WRL::ComPtr<ID3D12Device> device_;
	CommandQueue commandQueue_;
	CommandContext commandContext_;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_;
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_;
	SwapChain swapChain_;

	DepthBuffer mainDepthBuffer_;
	ColorBuffer mainColorBuffer_;

	PostEffect postEffect_;
	Bloom bloom_;

	DescriptorHeap descriptorHeaps_[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

	int32_t backBufferWidth_ = 0;
	int32_t backBufferHeight_ = 0;

private:
	void InitializeDXGIDevice();
	void CreateDirectXCompilier();
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource();
};

