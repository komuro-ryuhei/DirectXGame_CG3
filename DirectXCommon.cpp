//#include "DirectXCommon.h"
//
//#pragma comment(lib,"d3d12.lib")
//#pragma comment(lib,"dxgi.lib")
//
//void Log(const std::string& message) {
//
//	OutputDebugStringA(message.c_str());
//}
//
//void Log(const std::wstring& message) {
//	OutputDebugStringW(message.c_str());
//}
//
//DirectXCommon* DirectXCommon::GetInstance() {
//	static DirectXCommon instance;
//	return &instance;
//}
//
//void DirectXCommon::Initialize(WinApp* winApp) {
//
//	winApp_ = winApp;
//
//	// DXGIデバイス初期化
//	InitializeDXGIDevice();
//
//	// 
//	InitializeCommand();
//
//	// 
//	CreateSwapChain();
//
//	// 
//	DescriptorHeap();
//
//	// 
//	CreateRTV();
//
//	// 
//	FindBackBufferIndex();
//
//	// 
//	CreateFence();
//}
//
//void DirectXCommon::TransitionBarrir() {
//	// TransitionBarrierの設定
//	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
//
//	// Noneにしておく
//	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
//
//	// バリアを張る対象のリソース
//	barrier.Transition.pResource = swapChainResources[backBufferIndex];
//
//	// 遷移前のResourceState
//	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
//
//	// 遷移後のResourceState
//	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
//
//	// TransitionBarrierを張る
//	commandList->ResourceBarrier(1, &barrier);
//}
//
//void DirectXCommon::InitializeDXGIDevice() {
//
//#ifdef _DEBUG
//	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
//		// デバッグレイヤーを有効化する
//		debugController->EnableDebugLayer();
//
//		// さらにGPU側出もチェックを行えるようにする
//		debugController->SetEnableGPUBasedValidation(TRUE);
//	}
//
//#endif // DEBUG
//
//
//	dxgiFactory = nullptr;
//	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
//	assert(SUCCEEDED(hr));
//
//	useAdapter = nullptr;
//
//	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
//		IID_PPV_ARGS(&useAdapter)) != DXGI_ERROR_NOT_FOUND; i++) {
//		DXGI_ADAPTER_DESC3 adaperDesc{};
//		hr = useAdapter->GetDesc3(&adaperDesc);
//		assert(SUCCEEDED(hr));
//
//		if (!(adaperDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
//			Log(std::format(L"Use Adapter:{}\n", adaperDesc.Description));
//			break;
//		}
//		useAdapter = nullptr;
//	}
//
//	assert(useAdapter != nullptr);
//
//	device = nullptr;
//
//	D3D_FEATURE_LEVEL featureLevels[] = {
//		D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_0
//	};
//	const char* featureLevelStrings[] = { "12.2,","12.1","12.0" };
//
//	for (size_t i = 0; i < _countof(featureLevels); ++i) {
//		hr = D3D12CreateDevice(useAdapter, featureLevels[i], IID_PPV_ARGS(&device));
//
//		if (SUCCEEDED(hr)) {
//			Log(std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
//			break;
//		}
//	}
//
//	assert(device != nullptr);
//	Log("Complete create D3D12Device!!!\n");
//
//#ifdef _DEBUG
//
//	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
//		// ヤバイエラー時に止まる
//		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
//		// エラー時に止まる
//		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
//		// 警告時に止まる
//		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
//
//		// 抑制するメッセージのID
//		D3D12_MESSAGE_ID denyIds[] = {
//			// Window11でのDXGIデバックレイヤーとDX12デバックレイヤーの相互作用バグによるエラーメッセージ
//			// https://stackoverflow.com/questions/69805245/directx-12-application-is-crashing-in-window-11
//			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
//		};
//
//		// 抑制するレベル
//		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
//		D3D12_INFO_QUEUE_FILTER filter{};
//		filter.DenyList.NumIDs = _countof(denyIds);
//		filter.DenyList.pIDList = denyIds;
//		filter.DenyList.NumSeverities = _countof(severities);
//		filter.DenyList.pSeverityList = severities;
//
//		// 指定したメッセージの表示を抑制する
//		infoQueue->PushStorageFilter(&filter);
//		// 解放
//		infoQueue->Release();
//	}
//
//#endif // DEBUG
//
//}
//
//void DirectXCommon::InitializeCommand() {
//	HRESULT hr;
//
//	hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
//	assert(SUCCEEDED(hr));
//
//	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
//	assert(SUCCEEDED(hr));
//
//	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr,
//		IID_PPV_ARGS(&commandList));
//	assert(SUCCEEDED(hr));
//}
//
//void DirectXCommon::CreateSwapChain() {
//	HRESULT hr;
//
//	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
//	swapChainDesc.Width = kClientWidth_; // 画面の幅
//	swapChainDesc.Height = kClientHeight_; // 画面の高さ
//	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 色の形式
//	swapChainDesc.SampleDesc.Count = 1; // マルチサンプルしない
//	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 描画のターゲットとして利用
//	swapChainDesc.BufferCount = 2; // ダブルバッファ
//	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // モニタに移したら中身を吐き破棄
//
//	// コマンドキュー、ウィンドウハンドル、設定を渡して生成
//	hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue, winApp_->GetHwnd(), &swapChainDesc,
//		nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(&swapChain));
//}
//
//void DirectXCommon::DescriptorHeap() {
//	HRESULT hr;
//
//	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc{};
//	rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // レンダーターゲットビュー用
//	rtvDescriptorHeapDesc.NumDescriptors = 2; // ダブルバッファ用に2つ。多くても構わない
//	hr = device->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));
//	// ディスクリプタヒープが作れなかったので起動できない
//	assert(SUCCEEDED(hr));
//
//	// SwapChainからResourceを引っ張ってくる
//	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
//	// うまく取得できなければ起動できない
//	assert(SUCCEEDED(hr));
//	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
//	assert(SUCCEEDED(hr));
//}
//
//void DirectXCommon::CreateRTV() {
//
//	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // 出力結果をSRGBに変換して書き込む
//	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D; // 2Dテクスチャとして書き込む
//
//	// ディスクリプタの先端を取得する
//	rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
//
//	// まず1つ目を作る。1つ目は最初のところに作る。作る場所をこちらで指定する必要がある。
//	rtvHandles[0] = rtvStartHandle;
//	device->CreateRenderTargetView(swapChainResources[0], &rtvDesc, rtvHandles[0]);
//
//	// 2つ目のディクリプタハンドルを得る
//	rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
//	// 2つ目を作る
//	device->CreateRenderTargetView(swapChainResources[1], &rtvDesc, rtvHandles[1]);
//}
//
//void DirectXCommon::FindBackBufferIndex() {
//	HRESULT hr;
//
//	backBufferIndex = swapChain->GetCurrentBackBufferIndex();
//
//	TransitionBarrir();
//
//	// 描画先のRTVを設定する
//	commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, nullptr);
//
//	// 指定した色で画面全体をクリアする
//	float clearColor[] = { 0.1f,0.25f,0.5f,1.0f }; // 
//	commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);
//
//	// 画面に描く処理はすべて終わり、画面に移すので状態を遷移
//	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
//	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
//
//	// TransitionBarrierを張る
//	commandList->ResourceBarrier(1, &barrier);
//
//	// コマンドリストの内容を確定させる。
//	hr = commandList->Close();
//	assert(SUCCEEDED(hr));
//
//	ID3D12CommandList* commandLists[] = { commandList };
//	commandQueue->ExecuteCommandLists(1, commandLists);
//
//	// GPUとOSに画面の交換を行うよう通知する
//	swapChain->Present(1, 0);
//
//	// Fenceの値を更新
//	fenceValue++;
//
//	// GPUがここまでたどり着いたときに、Fenceの値を指定した値に代入するようにSignalを送る
//	commandQueue->Signal(fence, fenceValue);
//
//	// 次のフレーム用のコマンドリストを準備
//	hr = commandAllocator->Reset();
//	assert(SUCCEEDED(hr));
//	hr = commandList->Reset(commandAllocator, nullptr);
//	assert(SUCCEEDED(hr));
//
//	// Fenceの値が指定したSignalにたどり着いているか確認する
//	if (fence->GetCompletedValue() < fenceValue) {
//		// 指定したSignalにたどり着いていないので、たどり着くまで待つようにイベントを設定する
//		fence->SetEventOnCompletion(fenceValue, fenceEvent);
//
//		// イベントを待つ
//		WaitForSingleObject(fenceEvent, INFINITE);
//	}
//}
//
//void DirectXCommon::CreateFence() {
//	HRESULT hr;
//
//	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
//	assert(SUCCEEDED(hr));
//
//	// FenceのSignalを持つためのイベントを作成する
//	HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
//	assert(fenceEvent != nullptr);
//}