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

class DirectXCommon
{
public:

	//うんち設計
	uint32_t kSrvHeapDescritorNum = 1024;

	static DirectXCommon* GetInstance();
	void Initialize(int32_t backBufferWidth = 1280, int32_t backBufferHeight = 720);
	
	void PreDraw();
	void PostDraw();
	void ClearRenderTarget();
	void ClearDepthBuffer();

	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(const std::wstring& filePath,const wchar_t* profile);

	DescriptorHandle AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type);

	ID3D12Device* GetDevice() { return device_.Get(); }
	ID3D12GraphicsCommandList* GetCommandList() { return commandList_.Get(); }
	CommandQueue& GetCommandQueue() { return commandQueue_; }

	DescriptorHeap& GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type) { return descriptorHeaps_[type]; }
	
	
private:
	WinApp* winApp_;

	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_;
	Microsoft::WRL::ComPtr<ID3D12Device> device_;
	CommandQueue commandQueue_;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_;
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_;
	SwapChain swapChain_;
	Microsoft::WRL::ComPtr<ID3D12Resource> depthBuffer_;

	DescriptorHeap descriptorHeaps_[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

	int32_t backBufferWidth_ = 0;
	int32_t backBufferHeight_ = 0;

private:
	void InitializeDXGIDevice();
	void InitializeCommand();
	/*void CreateSwapChain();*/
	void CreateDirectXCompilier();
	/*void CreateFinalRenderTargets();*/
	void CreateDepthBuffer();
	void CreateSrvHeap();
	void TransitionResource(GPUResource& resource, D3D12_RESOURCE_STATES newState);
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource();
};

