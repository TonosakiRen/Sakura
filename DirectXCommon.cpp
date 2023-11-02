#include "DirectXCommon.h"
#include <assert.h>
#include "ConvertString.h"
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

using namespace Microsoft::WRL;

DirectXCommon* DirectXCommon::GetInstance() {
	static DirectXCommon instance;
	return &instance;
}

void DirectXCommon::Initialize(int32_t backBufferWidth, int32_t backBufferHeight) {

	assert(4 <= backBufferWidth && backBufferWidth <= 4096);
	assert(4 <= backBufferHeight && backBufferHeight <= 4096);

	winApp_ = WinApp::GetInstance();
	backBufferWidth_ = backBufferWidth;
	backBufferHeight_ = backBufferHeight;

	InitializeDXGIDevice();
	InitializeCommand();
	CreateDirectXCompilier();
	descriptorHeaps_[D3D12_DESCRIPTOR_HEAP_TYPE_RTV].Create(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, SwapChain::kNumBuffers);
	swapChain_.Create(winApp_->GetHwnd());
	CreateDepthBuffer();
	CreateSrvHeap();
	
}

void DirectXCommon::PreDraw() {

	////TransitionBarrierの設定
	//D3D12_RESOURCE_BARRIER barrier{};
	////今回のバリアはTransition
	//barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	////Noneにしておく
	//barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	////バリアを張る対象のリソース。現在のバックバッファに対して行う 
	//barrier.Transition.pResource = swapChain_.GetColorBuffer();
	////遷移前(現在)のResorceState
	//barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	////遷移後のResourceState
	//barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	////TransitionBarrierを張る
	//commandList_->ResourceBarrier(1, &barrier);

	TransitionResource(swapChain_.GetColorBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET);


	//// レンダーターゲットビュー用ディスクリプタヒープのハンドルを取得
	//CD3DX12_CPU_DESCRIPTOR_HANDLE rtvH = CD3DX12_CPU_DESCRIPTOR_HANDLE(
	//	rtvHeap_->GetCPUDescriptorHandleForHeapStart(), bbIndex,
	//	device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));

	// レンダーターゲットビュー用ディスクリプタヒープのハンドルを取得
	DescriptorHandle rtvH = descriptorHeaps_[D3D12_DESCRIPTOR_HEAP_TYPE_RTV].GetDiscriptorHandle(swapChain_.GetBufferIndex());

	// 深度ステンシルビュー用デスクリプタヒープのハンドルを取得
	/*CD3DX12_CPU_DESCRIPTOR_HANDLE dsvH =
		CD3DX12_CPU_DESCRIPTOR_HANDLE(dsvHeap_.GetCPUDescriptorHandleForHeapStart());*/

	// 深度ステンシルビュー用デスクリプタヒープのハンドルを取得
	DescriptorHandle dsvH = descriptorHeaps_[D3D12_DESCRIPTOR_HEAP_TYPE_DSV].GetDiscriptorHandle(0); //start

	// レンダーターゲットをセット
	commandList_->OMSetRenderTargets(1, &rtvH.GetCpuHandle(), false, &dsvH.GetCpuHandle());
	
	// 全画面クリア
	ClearRenderTarget();
	// 深度バッファクリア
	ClearDepthBuffer();

	// ビューポートの設定
	CD3DX12_VIEWPORT viewport =
		CD3DX12_VIEWPORT(0.0f, 0.0f, float(backBufferWidth_), float(backBufferHeight_));
	commandList_->RSSetViewports(1, &viewport);
	// シザリング矩形の設定
	CD3DX12_RECT rect = CD3DX12_RECT(0, 0, backBufferWidth_, backBufferHeight_);
	commandList_->RSSetScissorRects(1, &rect);

	//デスクリプターヒープを設定する
	ID3D12DescriptorHeap* ppHeaps[] = { descriptorHeaps_[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]};
	commandList_->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
}

void DirectXCommon::PostDraw() {
	HRESULT result = S_FALSE;

	//// リソースバリアを変更（描画対象→表示状態）
	//UINT bbIndex = swapChain_.GetBufferIndex();
	//CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
	//	backBuffers_[bbIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET,
	//	D3D12_RESOURCE_STATE_PRESENT);
	//commandList_->ResourceBarrier(1, &barrier);

	TransitionResource(swapChain_.GetColorBuffer(), D3D12_RESOURCE_STATE_PRESENT);

	// 命令のクローズ
	commandList_->Close();

	// コマンドリストの実行
	//ID3D12CommandList* cmdLists[] = { commandList_.Get() }; // コマンドリストの配列
	//commandQueue_->ExecuteCommandLists(1, cmdLists);

	commandQueue_.Execute(commandList_.Get());

	// バッファをフリップ
	swapChain_.Present();
#ifdef _DEBUG
	if (FAILED(result)) {
		ComPtr<ID3D12DeviceRemovedExtendedData> dred;

		result = device_->QueryInterface(IID_PPV_ARGS(&dred));
		assert(SUCCEEDED(result));

		// 自動パンくず取得
		D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT autoBreadcrumbsOutput{};
		result = dred->GetAutoBreadcrumbsOutput(&autoBreadcrumbsOutput);
		assert(SUCCEEDED(result));
	}
#endif

	// コマンドリストの実行完了を待つ
	/*commandQueue_->Signal(fence_.Get(), ++fenceVal_);
	if (fence_->GetCompletedValue() != fenceVal_) {
		HANDLE event = CreateEvent(nullptr, false, false, nullptr);
		fence_->SetEventOnCompletion(fenceVal_, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}*/

	commandQueue_.Signal();
	commandQueue_.WaitForGPU();

	commandAllocator_->Reset(); // キューをクリア
	commandList_->Reset(commandAllocator_.Get(),
		nullptr); // 再びコマンドリストを貯める準備
}

void DirectXCommon::ClearRenderTarget() {
	// レンダーターゲットビュー用ディスクリプタヒープのハンドルを取得
	DescriptorHandle rtvH = descriptorHeaps_[D3D12_DESCRIPTOR_HEAP_TYPE_RTV].GetDiscriptorHandle(swapChain_.GetBufferIndex());

	// 全画面クリア          Red   Green  Blue  Alpha
	float clearColor[] = { 0.1f, 0.25f, 0.5f, 0.0f }; // 青っぽい色
	commandList_->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);
}

void DirectXCommon::ClearDepthBuffer() {
	// 深度ステンシルビュー用デスクリプタヒープのハンドルを取得
	DescriptorHandle dsvH = descriptorHeaps_[D3D12_DESCRIPTOR_HEAP_TYPE_DSV].GetDiscriptorHandle(0);
	// 深度バッファのクリア
	commandList_->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}


void DirectXCommon::InitializeDXGIDevice() {
	HRESULT hr = S_FALSE;

#ifdef _DEBUG

	ComPtr<ID3D12Debug1> debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		//デバックレイヤーを有効化する	
		debugController->EnableDebugLayer();
		//さらにGPU側でもチェックを行えるようにする
		debugController->SetEnableGPUBasedValidation(TRUE);

	}

#endif

	//DXGIファクトリー	
	dxgiFactory_ = nullptr;
	//HRESULTはWIndows系のエラーコードであり、
	//関数が成功したかどうかをSUCCEEDEDマクロで判定できる
	hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_));
	//初期化の根本的な部分でエラーが出た場合はプログラングが間違っているか、
	//どうにもできない場合が多いのでassertにしておく
	assert(SUCCEEDED(hr));
	//使用するアダプタ用の変数、最初にnullptrを入れておく
	ComPtr<IDXGIAdapter4> useAdapter = nullptr;
	//良い順にアダプタを頼む
	for (UINT i = 0; dxgiFactory_->EnumAdapterByGpuPreference(i,
		DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) !=
		DXGI_ERROR_NOT_FOUND; ++i) {
		//アダプターの情報を取得する	
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr)); //取得できないのは一大事
		//ソフトウェアアダプタでなければ採用！
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			Log(std::format(L"Use Adapter{}\n", adapterDesc.Description));
			break;
		}
		useAdapter = nullptr; //ソフトウェアアダプタの場合は見なかったことにする
	}
	//適切なアダプタが見つからんかったので起動できない
	assert(useAdapter != nullptr);

	//D3D12Deviceの生成
	device_ = nullptr;
	//機能レベルとログ出力用の文字列
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0
	};
	const char* featureLevelStrings[] = { "12.2","12.1","12.0" };
	//高い順に生成できるか試していく
	for (size_t i = 0; i < _countof(featureLevels); ++i) {
		//採用したアダプターでデバイスを生成
		hr = D3D12CreateDevice(useAdapter.Get(), featureLevels[i], IID_PPV_ARGS(&device_));
		//指定した機能レベルでデバイスが生成できたか確認
		if (SUCCEEDED(hr)) {
			//生成できたのでログ出力を行ってループを抜ける	
			Log(std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
			break;
		}
	}
	//デバイスの生成がうまくいかなかったので起動できない
	assert(device_ != nullptr);
	Log("Complete create D3D12Device!!!\n");//初期化完了のログを出す

	const uint32_t desriptorSizeSRV = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	const uint32_t desriptorSizeRTV = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	const uint32_t desriptorSizeDSV = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
	if (SUCCEEDED(device_->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
		//やばいエラー前に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		//エラー時に泊まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		//警告時に泊まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		//抑制するメッセージのID
		D3D12_MESSAGE_ID denyIds[] = {
			//Windows11でのDXGIデバックレイヤーとDX12デバッグレイヤーの相方作用バグによるエラーメッセージ
			//https://stackoverflow.com/questions/69805245/directx-12-application-is-crashing-in-windows-11
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};
		//抑制するレベル
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		//指定したメッセージの表示を抑制する
		infoQueue->PushStorageFilter(&filter);
	}

}
void DirectXCommon::InitializeCommand() {
	HRESULT hr = S_FALSE;

	//コマンドキューを生成する
	//D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	//hr = device_->CreateCommandQueue(&commandQueueDesc,
	//	IID_PPV_ARGS(commandQueue_));
	////コマンドキューの生成がうまくいかなかったので起動できない
	//assert(SUCCEEDED(hr));

	commandQueue_.Create();

	//コマンドリストアロケータ生成をする
	commandAllocator_ = nullptr;
	hr = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator_));
	//コマンドアロケータの生成ががうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

	//コマンドリストを生成する
	commandList_ = nullptr;
	hr = device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_.Get(), nullptr,
		IID_PPV_ARGS(&commandList_));
	//コマンドリストの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));
}

void DirectXCommon::CreateDirectXCompilier() {
	HRESULT hr = S_FALSE;
	//dxCompilerを初期化
	dxcUtils_;
	dxcCompiler_;
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(dxcUtils_.GetAddressOf()));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(dxcCompiler_.GetAddressOf()));
	assert(SUCCEEDED(hr));

	//現時点でincludeはしないが、includeに対応するための設定を行っておく
	includeHandler_;
	hr = dxcUtils_->CreateDefaultIncludeHandler(&includeHandler_);
	assert(SUCCEEDED(hr));
}

ComPtr<IDxcBlob> DirectXCommon::CompileShader(
	//compilerするShaderファイルへのパス
	const std::wstring& filePath,
	//Compilerに使用するprofile
	const wchar_t* profile
	) {

	std::wstring directionary = L"./Resources/shaders/" + filePath;
	//これからシェーダーをコンパイルする旨をログに出す
	Log(ConvertString(std::format(L"Begin CompileShader,path:{},profile:{}\n", filePath, profile)));
	//hlslファイルを読む
	IDxcBlobEncoding* shaderSource = nullptr;
	HRESULT hr = dxcUtils_->LoadFile(directionary.c_str(), nullptr, & shaderSource);
	//読めなかったら止める
	assert(SUCCEEDED(hr));
	//読み込んだファイルの内容を設定する
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;//UTF8の文字コードであることを通知

	LPCWSTR arguments[] = {
		filePath.c_str(),//コンパイル対象のhlslファイル名
		L"-E",L"main",//エントリーポイントの指定。基本的にmain以外にはしない
		L"-T",profile,//ShaderProfileの設定
		L"-Zi",L"-Qembed_debug",//デバック用の情報を埋め込む
		L"-Od",//最適化を外しておく
		L"-Zpr",//メモリレイアウト行優先
	};

	//実際にshaderをコンパイルする
	IDxcResult* shaderResult = nullptr;
	hr = dxcCompiler_->Compile(
		&shaderSourceBuffer,//読み込んだファイル
		arguments,//コンパイルオプション
		_countof(arguments),//コンパイルオプションの数
		includeHandler_.Get(),//includeが含まれたもろもろ
		IID_PPV_ARGS(&shaderResult)//コンパイル結果

	);
	//コンパイルエラーではなくdncが起動できないなど致命的な状況
	assert(SUCCEEDED(hr));

	//警告・エラーが出てたらログに出して止める
	IDxcBlobUtf8* shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		std::string error = shaderError->GetStringPointer();
		Log(error);
		//警告・エラーダメ絶対
		assert(false);
	}

	//コンパイル結果から実行用のバイナリを取得
	IDxcBlob* shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));
	//成功したログを出す
	Log(ConvertString(std::format(L"Compile Succeeded, path:{} , profile:{}\n", filePath, profile)));
	//もう使わないリソース
	shaderSource->Release();
	shaderResult->Release();
	//実行用のバイナリを返却
	return shaderBlob;
}

DescriptorHandle DirectXCommon::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type) {
	return descriptorHeaps_[type].Allocate();
}

//void DirectXCommon::CreateSwapChain() {
//	HRESULT hr = S_FALSE;
//	
//	//スワップチェーンを生成する	
//	swapChain_ = nullptr;
//	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
//	swapChainDesc.Width = backBufferWidth_;//画面の幅。ウィンドウのクライアント領域を同じものにしておく
//	swapChainDesc.Height = backBufferHeight_;//画面の高さ。ウィンドウのクライアント領域を同じものにしておく
//	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//色の形式
//	swapChainDesc.SampleDesc.Count = 1;//マルチサンプルしない
//	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//描画のターゲットとして利用する
//	swapChainDesc.BufferCount = 2;//ダブルバッファ
//	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;//モニタにうつしたら、中身を破棄する
//	//コマンドキュー、ウィンドウハンドル、設定を渡して生成する	
//	hr = dxgiFactory_->CreateSwapChainForHwnd(commandQueue_, winApp_->GetHwnd(), &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain_.GetAddressOf()));
//	assert(SUCCEEDED(hr));
//
//	//SwapChainからResourceを引っ張てくる
//	backBuffers_.resize(swapChainDesc.BufferCount);
//	hr = swapChain_->GetBuffer(0, IID_PPV_ARGS(&backBuffers_[0]));
//	//うまく取得できなければ起動できない
//	assert(SUCCEEDED(hr));
//	hr = swapChain_->GetBuffer(1, IID_PPV_ARGS(&backBuffers_[1]));
//	assert(SUCCEEDED(hr));
//
//}
//void DirectXCommon::CreateFinalRenderTargets() {
//	/*rtvHeap_ = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);*/
//	descriptorHeaps_[D3D12_DESCRIPTOR_HEAP_TYPE_RTV].Create(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2);
//
//	DescriptorHandle rtvHandles[2];
//	rtvHandles[0] = descriptorHeaps_[D3D12_DESCRIPTOR_HEAP_TYPE_RTV].Allocate();
//	rtvHandles[1] = descriptorHeaps_[D3D12_DESCRIPTOR_HEAP_TYPE_RTV].Allocate();
//
//	//RTVの設定
//	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
//	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;//出力結果をSRGBに変換して書き込む
//	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;//2dtextureとして書き込む
//
//	device_->CreateRenderTargetView(backBuffers_[0].Get(), &rtvDesc, rtvHandles[0]);
//	device_->CreateRenderTargetView(backBuffers_[1].Get(), &rtvDesc, rtvHandles[1]);
//	
//
//	////RTVの設定
//	//D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
//	//rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;//出力結果をSRGBに変換して書き込む
//	//rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;//2dtextureとして書き込む
//	////ディスクリプタの先頭を取得する	
//	//D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvHeap_->GetCPUDescriptorHandleForHeapStart();
//	////RTVを2つつくるのでディスクリプタを2つ用意
//	//D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
//	////まず一つ目を作る１つ目は最初のところに作る。作る場所をこちらで指定してあげる必要がある
//	//rtvHandles[0] = rtvStartHandle;
//	//device_->CreateRenderTargetView(backBuffers_[0].Get(), &rtvDesc, rtvHandles[0]);
//	////２つ目のディスクリプタハンドルを得る(自力で)
//	//rtvHandles[1].ptr = rtvHandles[0].ptr + device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
//	////２つ目を作る
//	//device_->CreateRenderTargetView(backBuffers_[1].Get(), &rtvDesc, rtvHandles[1]);
//
//}
void DirectXCommon::CreateDepthBuffer() {
	/*dsvHeap_ = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);*/
	descriptorHeaps_[D3D12_DESCRIPTOR_HEAP_TYPE_DSV].Create(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);

	//DSVの設定
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//Format.基本的にはResourceに合わせる。
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;//2dTexture

	//DepthStencilTextureをウィンドウのサイズで作成
	depthBuffer_ = CreateDepthStencilTextureResource();

	//DSVHeapの先頭にDSVを作る
	device_->CreateDepthStencilView(depthBuffer_.Get(), &dsvDesc, descriptorHeaps_[D3D12_DESCRIPTOR_HEAP_TYPE_DSV].GetDiscriptorHandle(0));
}

void DirectXCommon::CreateSrvHeap() {
	/*srvHeap_ = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kSrvHeapDescritorNum, true);*/
	descriptorHeaps_[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].Create(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kSrvHeapDescritorNum);
}

void DirectXCommon::TransitionResource(GPUResource& resource, D3D12_RESOURCE_STATES newState)
{
	D3D12_RESOURCE_STATES oldState = resource.state_;

	if (newState != oldState) {
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = resource;
		barrier.Transition.StateBefore = oldState;
		barrier.Transition.StateAfter = newState;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		resource.state_ = newState;

		commandList_->ResourceBarrier(1, &barrier);
	}
}


ComPtr<ID3D12Resource> DirectXCommon::CreateDepthStencilTextureResource() {
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = backBufferWidth_;//Textureの幅
	resourceDesc.Height = backBufferHeight_;//Textureの高さ
	resourceDesc.MipLevels = 1;//mipmapの数
	resourceDesc.DepthOrArraySize = 1;//配列Textureの配列数
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//DepthStencilとして利用可能なフォーマット
	resourceDesc.SampleDesc.Count = 1;//DepthStencilとして利用可能なフォーマット
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;//2次元
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;//DepthStencilとして使う

	//利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;//VRAM上に作る

	//深度値のクリア設定
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;//1.0f(最大値)でクリア
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//フォーマット。Resourceと合わせる

	//Resourceの生成
	ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device_->CreateCommittedResource(
		&heapProperties,//Heapの設定
		D3D12_HEAP_FLAG_NONE,//Heapの特殊な設定。特になし。
		&resourceDesc, //Resoruceの設定
		D3D12_RESOURCE_STATE_DEPTH_WRITE,//深度値を書き込む状態にしておく
		&depthClearValue, //Clear最適値
		IID_PPV_ARGS(&resource));//作成するResourceポインタへのポインタ
	assert(SUCCEEDED(hr));
	return resource;
}

ComPtr<ID3D12DescriptorHeap> DirectXCommon::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible) {
	ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HRESULT hr = device_->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	assert(SUCCEEDED(hr));
	return descriptorHeap;
}