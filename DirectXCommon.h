//#pragma once
//#include <cstdint>
//#include <string>
//#include <format>
//#include <cassert>
//#include <Windows.h>
//#include <cstdint>
//
//#include <d3d12.h>
//#include <dxgi1_6.h>
//
//#include "WinApp.h"
//
//class DirectXCommon {
//
//public:
//
//	static DirectXCommon* GetInstance();
//
//	void Initialize(WinApp* winApp);
//
//private: // メンバ変数
//	 ウィンドウズアプリケーション管理
//	WinApp* winApp_;
//
//	 
//	uint32_t kClientWidth_;
//	uint32_t kClientHeight_;
//
//	 
//	IDXGIFactory7* dxgiFactory = nullptr;
//
//	 
//	IDXGIAdapter4* useAdapter = nullptr;
//
//	 
//	ID3D12Device* device = nullptr;
//
//	
//	ID3D12CommandQueue* commandQueue = nullptr;
//
//	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
//
//	
//	ID3D12CommandAllocator* commandAllocator = nullptr;
//	ID3D12GraphicsCommandList* commandList = nullptr;
//
//	 スワップチェーンを生成
//	IDXGISwapChain4* swapChain = nullptr;
//
//	 ディスクリプタヒープの生成
//	ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;
//
//	 SwapChainからResourceを持ってくる
//	ID3D12Resource* swapChainResources[2] = { nullptr };
//
//	 RTVの設定
//	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
//
//	 ディスクリプタの先端を取得する
//	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle;
//	 RTVを2つ作るのでディスクリプタを2つ用意
//	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
//
//	 書き込むバックバッファのインデックスを取得
//	UINT backBufferIndex;
//
//	 
//	ID3D12Debug1* debugController = nullptr;
//
//	 
//	ID3D12InfoQueue* infoQueue = nullptr;
//
//	
//	D3D12_RESOURCE_BARRIER barrier{};
//
//	 
//	ID3D12Fence* fence = nullptr;
//	uint64_t fenceValue = 0;
//	HANDLE fenceEvent;
//
//private: // メンバ関数
//
//	DirectXCommon() = default;
//	~DirectXCommon() = default;
//
//	 <summary>
//	 DXGIデバイス初期化
//	 </summary>
//	void InitializeDXGIDevice();	
//
//	 <summary>
//	 コマンド初期化
//	 </summary>
//	void InitializeCommand();
//
//	 <summary>
//	 スワップチェーンの生成
//	 </summary>
//	void CreateSwapChain();
//
//	 <summary>
//	 ディスクリプタヒープの生成
//	 </summary>
//	void DescriptorHeap();
//
//	 <summary>
//	 RTVの作成
//	 </summary>
//	void CreateRTV();
//
//	 <summary>
//	 バックバッファのインデックスを取得
//	 </summary>
//	void FindBackBufferIndex();
//
//	
//	void TransitionBarrir();
//
//	void DebugLayer();
//
//	 <summary>
//	 Fenceの作成
//	 </summary>
//	void CreateFence();
//};