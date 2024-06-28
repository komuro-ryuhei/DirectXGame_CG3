#include <Windows.h>

#include "System.h"
#include <cstdint>
#include <string>
#include <format>
#include <cassert>
#include <wrl.h>

#include "MyMath.h"

#include "fstream"
#include "sstream"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <dxcapi.h>

#include "eternals/imgui/imgui.h"
#include "eternals/imgui/imgui_impl_dx12.h"
#include "eternals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxcompiler.lib")

#include <DirectXTex.h>

const char kWindowTitle[] = "LE2B_11_コムロ_リュウヘイ";

struct D3DResourceLeakChecker {
	~D3DResourceLeakChecker()
	{
		// リソースリークチェック
		Microsoft::WRL::ComPtr<IDXGIDebug1> debug;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
			debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
		}
	}
};

/*/////////////////////////////////////////////////////////////////////////////
		Log関数
*//////////////////////////////////////////////////////////////////////////////
void Log(const std::string& message) {

	OutputDebugStringA(message.c_str());
}

void Log(const std::wstring& message) {

	OutputDebugStringW(message.c_str());
}

/*/////////////////////////////////////////////////////////////////////////////
	ConvertString関数
*//////////////////////////////////////////////////////////////////////////////
std::wstring ConvertString(const std::string& str) {
	if (str.empty()) {
		return std::wstring();
	}

	auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), NULL, 0);
	if (sizeNeeded == 0) {
		return std::wstring();
	}
	std::wstring result(sizeNeeded, 0);
	MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), &result[0], sizeNeeded);
	return result;
}

std::string ConvertString(const std::wstring& str) {
	if (str.empty()) {
		return std::string();
	}

	auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0, NULL, NULL);
	if (sizeNeeded == 0) {
		return std::string();
	}
	std::string result(sizeNeeded, 0);
	WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded, NULL, NULL);
	return result;
}

/*/////////////////////////////////////////////////////////////////////////////
	pileShader関数
*//////////////////////////////////////////////////////////////////////////////
Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(
	// CompilerするShaderファイルへパス
	const std::wstring& filePath,
	// compilerに使用するprofile
	const wchar_t* profile,
	// 初期化で生成したものを3つ
	IDxcUtils* dxcUtils,
	IDxcCompiler3* dxcCompiler,
	IDxcIncludeHandler* includeHandler) {

	/////////////// hlslファイルを読む
	// これからシェーダーをコンパイルする旨をログに出す
	Log(ConvertString(std::format(L"Begin CompilerShader, path:{},profile:{}\n", filePath, profile)));
	// hlslファイルを読む
	Microsoft::WRL::ComPtr<IDxcBlobEncoding> shaderSource = nullptr;
	HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	// 読めなかったら止める
	assert(SUCCEEDED(hr));
	// 読み込んだファイルの内容を設定する
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;

	/////////////// Compileする
	LPCWSTR arguments[] = {
		filePath.c_str(), // コンパイル対象のhlslファイル名
		L"-E",L"main", // エントリーポイントの指定。基本的にmain以外はしない
		L"-T",profile, // ShaderProfileの指定
		L"-Zi",L"Qembed_debug", // デバッグ用の情報を埋め込む
		L"-Od", // 最適解を外しておく
		L"-Zpr", // メモリレイアウトは行優先
	};
	// 実際にShaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcResult> shaderResult = nullptr;
	hr = dxcCompiler->Compile(
		&shaderSourceBuffer, // 読み込んだファイル
		arguments, // コンパイルオプション
		_countof(arguments), // コンパイルオプションの数
		includeHandler, // includeが含まれた諸々
		IID_PPV_ARGS(&shaderResult) // コンパイル結果
	);
	// コンパイルエラーではなくdxcが起動できないなど致命的な状況
	assert(SUCCEEDED(hr));

	/////////////// 警告・エラーが出ていないか確認する
	// 警告・エラーが出てたらログに出して止める
	Microsoft::WRL::ComPtr<IDxcBlobUtf8> shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		Log(shaderError->GetStringPointer());
		// 警告・エラーダメゼッタイ
		assert(false);
	}

	/////////////// Compile結果を受け取って返す
	// コンパイル結果から実行用のバイナリ部分を取得
	Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));
	// 成功したらログを出す
	Log(ConvertString(std::format(L"Compile Succeeded, path:{}, profile:{}\n", filePath, profile)));
	// 実行用のバイナリを返却
	return shaderBlob;
}

/*/////////////////////////////////////////////////////////////////////////////
	ウィンドウプロシージャ
*//////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}

	// メッセージに応じてゲーム固有の処理
	switch (msg) {
		// ウインドウが破棄された
	case WM_DESTROY:
		// OSに対してアプリの終了を伝える
		PostQuitMessage(0);
		return 0;
	}
	// 標準のメッセージ処理を行う
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

/*/////////////////////////////////////////////////////////////////////////////
DescriptorHeapの関数化
*//////////////////////////////////////////////////////////////////////////////
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(
	ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heaptype, UINT numDescriptors, bool shaderVisible) {

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heaptype;
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	assert(SUCCEEDED(hr));
	return descriptorHeap;
}


/*/////////////////////////////////////////////////////////////////////////////
	Resource作成の関数化
*//////////////////////////////////////////////////////////////////////////////
Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(ID3D12Device* device, size_t sizeInBytes) {
	HRESULT hr;

	/*///////////////////////
		VertexResourceを生成
	*////////////////////////
	// 頂点リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	// 頂点リソースの設定
	D3D12_RESOURCE_DESC vertexResourceDesc{};
	// バッファリソース
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResourceDesc.Width = sizeInBytes; // リソースのサイズ。今回はVector4を3頂点分
	// バッファの場合はこれらは1にする決まり
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;
	// バッファの場合はこれにする決まり
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	// 実際に頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = nullptr;
	hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&vertexResource));
	assert(SUCCEEDED(hr));

	return vertexResource;
}

DirectX::ScratchImage LoadTexture(const std::string& filePath) {

	// テクスチャファイルを読んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	// ミニマップの作成
	DirectX::ScratchImage mipImage{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImage);
	assert(SUCCEEDED(hr));

	// ミニマップ付きのデータを渡す
	return mipImage;
}

Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata) {

	// metadaraを基にResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(metadata.width); // Textureの幅
	resourceDesc.Height = UINT(metadata.height); // Textureの高さ
	resourceDesc.MipLevels = UINT16(metadata.mipLevels); // mipmapの数
	resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize); // 奥行き or 配列Textureの配列
	resourceDesc.Format = metadata.format; // TextureのFormat
	resourceDesc.SampleDesc.Count = 1; // サンプリングカウント
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension); // Textureの次元数

	// 利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_CUSTOM; // 細かい設定を行う
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK; // WriteBackポリシーでCPUアクセス可能
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0; // プロセッサの近くに配置

	// Resourceの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties, // Heapの設定
		D3D12_HEAP_FLAG_NONE, // Heapの特殊な設定
		&resourceDesc, // Resourceの設定
		D3D12_RESOURCE_STATE_GENERIC_READ, // 初回のResourceState
		nullptr, // Clear最速値
		IID_PPV_ARGS(&resource)); // 作成するResourceポインタへのポインタ
	assert(SUCCEEDED(hr));
	return resource;
}

void UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages) {

	// Meta情報を取得
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	// 全MipMapについて
	for (size_t mipLevel = 0; mipLevel < metadata.mipLevels; ++mipLevel) {
		// MipMapLevelを指定して各Imageを取得
		const DirectX::Image* img = mipImages.GetImage(mipLevel, 0, 0);
		// Textureに転送
		HRESULT hr = texture->WriteToSubresource(
			UINT(mipLevel),
			nullptr, // 全領域へコピー
			img->pixels, // 元データへアクセス
			UINT(img->rowPitch), // 1ラインサイズ
			UINT(img->slicePitch) // 1枚サイズ
		);
		assert(SUCCEEDED(hr));
	}
}

Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(ID3D12Device* device, int32_t width, int32_t height) {

	// 生成するResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width; // Textureの幅
	resourceDesc.Height = height; // Textureの高さ
	resourceDesc.MipLevels = 1; // mipmapの数
	resourceDesc.DepthOrArraySize = 1; // 
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // DepthStencilとして利用可能なフォーマット
	resourceDesc.SampleDesc.Count = 1; // サンプリングカウント
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // 2次元
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; // DepthStencilとして使う通知

	// 利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // VRAM上に作る

	// 深度値のクリア設定
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f; //1.0f(1最大値)でクリア
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // フォーマット

	// Resourceの生成 ////////////////// コメント書いとけ
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
	return resource;
}

D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index) {

	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);

	return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index) {

	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);

	return handleGPU;
}

struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

struct MaterialData {
	std::string textureFilePath;
};

MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {

	// 構築するMatrialData
	MaterialData materialData;
	// ファイルから選んだ1行を格納するもの
	std::string line;
	// ファイルを開く
	std::ifstream file(directoryPath + "/" + filename);
	// 開けなかったら止める
	assert(file.is_open());

	while (std::getline(file, line)) {
		std::string identifer;
		std::istringstream s(line);
		s >> identifer;

		// identiferに応じた処理
		if (identifer == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;
			// 連結してファイルパスにする
			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}
	}

	return materialData;
}

struct ModelData {
	std::vector<VertexData> vertices;
	MaterialData material;
};

// obj読み込み
ModelData LoadOBJFile(const std::string& directoryPath, const std::string& filename) {

	ModelData modelData; // 構築するモデルデータ
	std::vector<Vector4> positions; // 位置
	std::vector<Vector3> normals; // 法線
	std::vector<Vector2> texcoords; // テクスチャ座標
	std::string line; // ファイルから読んだ1行を格納するもの

	std::ifstream file(directoryPath + "/" + filename); // ファイルを開く
	assert(file.is_open());

	// ファイルを読み、ModelDataを構築
	while (std::getline(file, line)) {
		std::string identifer;
		std::istringstream s(line);
		s >> identifer; // 先頭の識別子を読む

		// identiferに応じた処理
		if (identifer == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			// position.x *= -1;
			position.w = 1.0f;
			positions.push_back(position);
		} else if (identifer == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoord.y = 1.0f - texcoord.y;
			texcoords.push_back(texcoord);
		} else if (identifer == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normal.x *= -1;
			normals.push_back(normal);
		} else if (identifer == "f") {
			// 面は三角形限定
			VertexData triangle[3];
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;
				// 頂点の要素へのIndexは「位置/UV/法線」で格納されているので、分解してIndexを取得する
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (uint32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/'); // 区切りでインデックスを読んでいく
					elementIndices[element] = std::stoi(index);
				}
				// 要素へのIndexから、実際の要素の値を取得して、頂点を構築する
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];
				VertexData vertex = { position,texcoord,normal };
				modelData.vertices.push_back(vertex);
				triangle[faceVertex] = { position,texcoord,normal };
			}
			// 頂点を逆順で登録することで周り順を逆にする
			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);
		} else if (identifer == "mtllib") {
			// materialTempleteLibraryファイルの名前を取得する
			std::string materialFilename;
			s >> materialFilename;
			// 基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}

	return modelData;
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	D3DResourceLeakChecker leakCheck;

	// COMの初期化
	CoInitializeEx(0, COINIT_MULTITHREADED);


	WNDCLASS wc{};
	// ウインドウプロシージャ
	wc.lpfnWndProc = WindowProc;
	// ウインドウクラス名
	wc.lpszClassName = L"CG2WindowClass";
	// インスタンスハンドル
	wc.hInstance = GetModuleHandle(nullptr);
	//カーソル
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	// ウインドウクラスを登録する
	RegisterClass(&wc);

	// クライアント領域のサイズ
	static const int32_t kWindowWidth = 1280;
	static const int32_t kWindowHeight = 720;

	// ウインドウサイズを表す構造体にクライアント領域を入れる
	RECT wrc = { 0,0,kWindowWidth,kWindowHeight };

	// クライアント領域を元に実際のサイズにwrcを変更してもらう
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	// ウインドの生成
	HWND hwnd = CreateWindow(
		wc.lpszClassName,
		L"CG2",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		wc.hInstance,
		nullptr
	);

	// ウインドウを表示する
	ShowWindow(hwnd, SW_SHOW);

	// DebugLayer
#ifdef _DEBUG

	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		// デバッグレイヤーを有効化する
		debugController->EnableDebugLayer();

		// さらにGPU側出もチェックを行えるようにする
		debugController->SetEnableGPUBasedValidation(TRUE);
	}

#endif // DEBUG

	/*/////////////////////////////////////////////////////////////////////////////
		DXGIファクトリーの生成
	*//////////////////////////////////////////////////////////////////////////////
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory = nullptr;
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	assert(SUCCEEDED(hr));

	// 使用するアダプターの変数	
	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter = nullptr;

	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
		IID_PPV_ARGS(&useAdapter)) != DXGI_ERROR_NOT_FOUND; ++i) {
		// アダプターの情報を取得
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			// 採用したアダプタの情報をログに出力
			Log(std::format(L"Use Adapter:{}\n", adapterDesc.Description));
			break;
		}
		useAdapter = nullptr; // ソフトウェアアダプタの場合は見なかったことにする
	}
	// 適切なアダプタが見つからなかったので起動できない
	assert(useAdapter != nullptr);

	// D3D12Deviceの生成
	Microsoft::WRL::ComPtr<ID3D12Device> device = nullptr;
	// 機能レベルとログ出力用の文字列
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_0
	};
	const char* featureLevelStrings[] = { "12.2,","12.1","12.0" };
	// 高い順に生成できたか試していく
	for (size_t i = 0; i < _countof(featureLevels); ++i) {
		// 採用したアダプターでデバイスを生成
		hr = D3D12CreateDevice(useAdapter.Get(), featureLevels[i], IID_PPV_ARGS(&device));
		// 指定した機能レベルでデバイスが生成できたか確認
		if (SUCCEEDED(hr)) {
			// 生成できたのでログ出力を行ってループを抜ける
			Log(std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
			break;
		}
	}
	// デバイスの生成がうまくいかなかったので起動できない
	assert(device != nullptr);
	Log("Complete create D3D12Device!!!\n");

#ifdef _DEBUG

	Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue = nullptr;

	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
		// ヤバイエラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		// エラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		// 警告時に止まる
		//infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		// 抑制するメッセージのID
		D3D12_MESSAGE_ID denyIds[] = {
			// Window11でのDXGIデバックレイヤーとDX12デバックレイヤーの相互作用バグによるエラーメッセージ
			// https://stackoverflow.com/questions/69805245/directx-12-application-is-crashing-in-window-11
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};

		// 抑制するレベル
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;

		// 指定したメッセージの表示を抑制する
		infoQueue->PushStorageFilter(&filter);
	}

#endif // DEBUG

	/*/////////////////////////////////////////////////////////////////////////////
		コマンドキューを生成
	*//////////////////////////////////////////////////////////////////////////////

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};

	hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	// コマンドキューの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

	/*/////////////////////////////////////////////////////////////////////////////
		コマンドアロケータを生成
	*//////////////////////////////////////////////////////////////////////////////
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	assert(SUCCEEDED(hr));

	/*/////////////////////////////////////////////////////////////////////////////
		コマンドリストを生成
	*//////////////////////////////////////////////////////////////////////////////
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr,
		IID_PPV_ARGS(&commandList));
	assert(SUCCEEDED(hr));

	/*/////////////////////////////////////////////////////////////////////////////
		スワップチェインを生成
	*//////////////////////////////////////////////////////////////////////////////
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = kWindowWidth; // 画面の幅
	swapChainDesc.Height = kWindowHeight; // 画面の高さ
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 色の形式
	swapChainDesc.SampleDesc.Count = 1; // マルチサンプルしない
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 描画のターゲットとして利用
	swapChainDesc.BufferCount = 2; // ダブルバッファ
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // モニタに移したら中身を吐き破棄

	// コマンドキュー、ウィンドウハンドル、設定を渡して生成
	hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), hwnd, &swapChainDesc,
		nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain.GetAddressOf()));
	assert(SUCCEEDED(hr));


	// DescriptorSize
	const uint32_t descriptorSizeSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	const uint32_t descriptorSizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	const uint32_t descriptorSizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	/*/////////////////////////////////////////////////////////////////////////////
	ディスクリプターヒープを生成
	*//////////////////////////////////////////////////////////////////////////////
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap = CreateDescriptorHeap(device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
	// SRV用のヒープでディスクリプタの数は128
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap = CreateDescriptorHeap(device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);

	/*/////////////////////////////////////////////////////////////////////////////
	ディスクリプターヒープを生成
	*//////////////////////////////////////////////////////////////////////////////

	// SwapChainからResourceを引っ張ってくる
	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources[2] = { nullptr };
	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
	// うまく取得できなければ起動できない
	assert(SUCCEEDED(hr));
	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
	assert(SUCCEEDED(hr));

	/*/////////////////////////////////////////////////////////////////////////////
			RTVの設定
	*//////////////////////////////////////////////////////////////////////////////
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // 出力結果をSRGBに変換して書き込む
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D; // 2Dテクスチャとして書き込む

	// ディスクリプタの先端を取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = GetCPUDescriptorHandle(rtvDescriptorHeap, descriptorSizeRTV, 0);

	// RTVを2つ作るのでディスクリプタを2つ用意
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];

	// まず1つ目を作る。1つ目は最初のところに作る。作る場所をこちらで指定する必要がある。
	rtvHandles[0] = rtvStartHandle;
	device->CreateRenderTargetView(swapChainResources[0].Get(), &rtvDesc, rtvHandles[0]);

	// 2つ目のディクリプタハンドルを得る
	rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	// 2つ目を作る
	device->CreateRenderTargetView(swapChainResources[1].Get(), &rtvDesc, rtvHandles[1]);

	/*/////////////////////////////////////////////////////////////////////////////
		FenceとEventの生成
	*//////////////////////////////////////////////////////////////////////////////
	// 初期値0でFenceを作る
	Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;
	uint64_t fenceValue = 0;

	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(hr));

	// FenceのSignalを持つためのイベントを作成する
	HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent != nullptr);

	/*/////////////////////////////////////////////////////////////////////////////
			DXCの初期化
	*//////////////////////////////////////////////////////////////////////////////
	// dxcCompilerを初期化
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils = nullptr;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler = nullptr;
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(hr));

	// 
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler = nullptr;
	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));

	/*/////////////////////////////////////////////////////////////////////////////
			DepthStencilView(DSV)
	*//////////////////////////////////////////////////////////////////////////////
	// DepthStencilTextureをウィンドウのサイズで作成
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource = CreateDepthStencilTextureResource(device.Get(), kWindowWidth, kWindowHeight);

	// DSV用のヒープディスクリプタの数は1
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap = CreateDescriptorHeap(device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

	// DSV上設定
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // Format
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; // 2dTexture
	// DSVHeapの先頭にDSVを作る
	device->CreateDepthStencilView(depthStencilResource.Get(), &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	// 描画先のRTVとDSVを設定する
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = GetCPUDescriptorHandle(dsvDescriptorHeap, descriptorSizeDSV, 0);

	/*/////////////////////////////////////////////////////////////////////////////
		RootSignatureを生成
	*//////////////////////////////////////////////////////////////////////////////
	// RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	/*/////////////////////////////////////////////////////////////////////////////
		DescriptorRange
	*//////////////////////////////////////////////////////////////////////////////
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0; // 0から始まる
	descriptorRange[0].NumDescriptors = 1; // 数は1つ
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRVを使う
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // Offsetを自動計算


	// RootParameter作成
	D3D12_ROOT_PARAMETER rootParameters[4] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBVを使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
	rootParameters[0].Descriptor.ShaderRegister = 0; // レジスタ番号0とバインド

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBVを使う
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // VertexShaderで使う
	rootParameters[1].Descriptor.ShaderRegister = 0; // レジスタ番号0を使う

	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // DescriptorTableを使う
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange; // Tableの中身の配列を指定
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange); // Tableで利用する数

	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBVを使う
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
	rootParameters[3].Descriptor.ShaderRegister = 1; // レジスタ番号1を使う

	descriptionRootSignature.pParameters = rootParameters; // ルートパラメータ配列へポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters); // 配列の長さ

	/*/////////////////////////////////////////////////////////////////////////////
		Samplerの設定
	*//////////////////////////////////////////////////////////////////////////////
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // バイリニアフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // 0~1の範囲外をリピート
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; // 比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX; // ありったけのMipmapを使う
	staticSamplers[0].ShaderRegister = 0; // レジスタ番号0を使う
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);


	// シリアライズしてバイナリにする
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	// バイナリを元に生成
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	// InputLayer
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	// BlendStateの設定
	D3D12_BLEND_DESC blendDesc{};
	// 全ての色要素を書き込む
// ブレンドモードNone D3D12_COLOR_WRITE_ENABLE_ALLだけ
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

	// ResiterzerStateの設定
	D3D12_RASTERIZER_DESC rasterizeDesc{};
	// 裏面(時計回り)を表示しない
	rasterizeDesc.CullMode = D3D12_CULL_MODE_BACK;
	// 三角形の中を塗りつぶす
	rasterizeDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// Shaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = CompileShader(L"Object3D.VS.hlsl",
		L"vs_6_0", dxcUtils.Get(), dxcCompiler.Get(), includeHandler.Get());
	assert(vertexShaderBlob != nullptr);

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = CompileShader(L"Object3D.PS.hlsl",
		L"ps_6_0", dxcUtils.Get(), dxcCompiler.Get(), includeHandler.Get());
	assert(pixelShaderBlob != nullptr);

	// DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	// Depthの機能を有効化する
	depthStencilDesc.DepthEnable = true;
	// 書き込みします
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	// 比較関数はLessEqual
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	/*/////////////////////////////////////////////////////////////////////////////
			PSOを生成
	*//////////////////////////////////////////////////////////////////////////////
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature.Get(); // RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc; // InputLayout
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
	vertexShaderBlob->GetBufferSize() }; // VertexShader
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
	pixelShaderBlob->GetBufferSize() }; // PixelShader
	graphicsPipelineStateDesc.BlendState = blendDesc; // BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizeDesc; // RasterizerState

	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	// 利用するトポロジ(形状)のタイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	// どのように画面に色を打ち込むのか設定
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	// DepthStencilの設定
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// 実際に生成
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
	hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));

	///*/////////////////////////////////////////////////////////////////////////////
	//	VertexResourceを生成
	//*//////////////////////////////////////////////////////////////////////////////

	/*struct VertexData {
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;
	};*/

	struct Material {
		Vector4 color;
		bool enableLighting;
		float padding[3];
		Matrix4x4 uvTransform;
	};

	struct DirectionalLight {
		Vector4 color; // ライトの色
		Vector3 direction; // ライトの向き
		float intensity; // 輝度
	};

	struct Transform {

		Vector3 scale;
		Vector3 rotate;
		Vector3 translate;
	};

	struct TransformationMatrix {
		Matrix4x4 WVP;
		Matrix4x4 World;
	};

	// 分割数
	uint32_t kSubdivision = 16;

	ModelData modelData = LoadOBJFile("Resources", "plane.obj");

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = CreateBufferResource(device.Get(), sizeof(VertexData) * modelData.vertices.size());

	/*/////////////////////////////////////////////////////////////////////////////
		VertexBufferViewを生成
	*//////////////////////////////////////////////////////////////////////////////
	// 頂点バッファビューを作成
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	// リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	// 1頂点当あたりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);


	/*/////////////////////////////////////////////////////////////////////////////
		3角形の頂点の情報
	*//////////////////////////////////////////////////////////////////////////////
	// 頂点リソースにデータを書き込む
	VertexData* vertexData = nullptr;
	// 書き込むアドレスを取得
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	// 頂点データにリソースをコピー
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());

	// 左下
	//vertexData[0].position = { -0.5f,-0.5f,0.0f,1.0f };
	//vertexData[0].texcoord = { 0.0f,1.0f };
	//// 上
	//vertexData[1].position = { 0.0f,0.5f,0.0f,1.0f };
	//vertexData[1].texcoord = { 0.5f,0.0f };
	//// 右下
	//vertexData[2].position = { 0.5f,-0.5f,0.0f,1.0f };
	//vertexData[2].texcoord = { 1.0f,1.0f };

	//// 左下2
	//vertexData[3].position = { -0.5f,-0.5f,0.5f,1.0f };
	//vertexData[3].texcoord = { 0.0f,1.0f };
	//// 上2
	//vertexData[4].position = { 0.0f,0.0f,0.0f,1.0f };
	//vertexData[4].texcoord = { 0.5f,0.0f };
	//// 右下2
	//vertexData[5].position = { 0.5f,-0.5f,-0.5f,1.0f };
	//vertexData[5].texcoord = { 1.0f,1.0f };


	/*/////////////////////////////////////////////////////////////////////////////
		スプライトの頂点の情報
	*//////////////////////////////////////////////////////////////////////////////
	// Sprite用の頂点リソースの作成
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSprite = CreateBufferResource(device.Get(), sizeof(VertexData) * 4);


	// 頂点バッファービューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
	// リソース用の先頭のアドレスから使う
	vertexBufferViewSprite.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点6つ分のサイズ
	vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 4;
	// 1頂点当たりのサイズ
	vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);

	VertexData* vertexDataSprite = nullptr;
	vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));

	// 1枚目の三角形
	vertexDataSprite[0].position = { 0.0f,360.0f,0.0f,1.0f }; // 左下
	vertexDataSprite[0].texcoord = { 0.0f,1.0f };
	vertexDataSprite[0].normal = { 0.0f,0.0f,-1.0f };

	vertexDataSprite[1].position = { 0.0f,0.0f,0.0f,1.0f }; // 左上
	vertexDataSprite[1].texcoord = { 0.0f,0.0f };
	vertexDataSprite[1].normal = { 0.0f,0.0f,-1.0f };

	vertexDataSprite[2].position = { 640.0f,360.0f,0.0f,1.0f }; // 右下
	vertexDataSprite[2].texcoord = { 1.0f,1.0f };
	vertexDataSprite[2].normal = { 0.0f,0.0f,-1.0f };

	vertexDataSprite[3].position = { 640.0f,0.0f,0.0f,1.0f }; // 右上
	vertexDataSprite[3].texcoord = { 1.0f,0.0f };
	vertexDataSprite[3].normal = { 0.0f,0.0f,-1.0f };

	// 2枚目の三角形
	//vertexDataSprite[3].position = { 640.0f,0.0f,0.0f,1.0f }; // 右上
	//vertexDataSprite[3].texcoord = { 1.0f,0.0f };
	//vertexDataSprite[3].normal = { 0.0f,0.0f,-1.0f };

	//vertexDataSprite[4].position = { 0.0f,0.0f,0.0f,1.0f }; // 左上
	//vertexDataSprite[4].texcoord = { 0.0f,0.0f };
	//vertexDataSprite[4].normal = { 0.0f,0.0f,-1.0f };

	//vertexDataSprite[5].position = { 640.0f,360.0f,0.0f,1.0f }; // 右下
	//vertexDataSprite[5].texcoord = { 1.0f,1.0f };
	//vertexDataSprite[5].normal = { 0.0f,0.0f,-1.0f };

	// Index用
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResourceSprite = CreateBufferResource(device.Get(), sizeof(uint32_t) * 6);

	// IBV
	D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};
	// リソースの先頭アドレスから使う
	indexBufferViewSprite.BufferLocation = indexResourceSprite->GetGPUVirtualAddress();
	// 使用するリソースのサイズはインデックス6つ分のサイズ
	indexBufferViewSprite.SizeInBytes = sizeof(uint32_t) * 6;
	// インデックスはuint32_tとする
	indexBufferViewSprite.Format = DXGI_FORMAT_R32_UINT;


	// インデックスリソースにデータを書き込む
	uint32_t* indexDataSprite = nullptr;
	indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));

	indexDataSprite[0] = 0;
	indexDataSprite[1] = 1;
	indexDataSprite[2] = 2;

	indexDataSprite[3] = 1;
	indexDataSprite[4] = 3;
	indexDataSprite[5] = 2;

	/*/////////////////////////////////////////////////////////////////////////////
		球の頂点の情報
	*//////////////////////////////////////////////////////////////////////////////
	// Sphere用の頂点リソースの作成
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSphere = CreateBufferResource(device.Get(), sizeof(VertexData) * 1536);
	// 頂点バッファービューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSphere{};
	// リソース用の先頭のアドレスから使う
	vertexBufferViewSphere.BufferLocation = vertexResourceSphere->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点6つ分のサイズ
	vertexBufferViewSphere.SizeInBytes = sizeof(VertexData) * 1536;
	// 1頂点当たりのサイズ
	vertexBufferViewSphere.StrideInBytes = sizeof(VertexData);

	VertexData* vertexDataSphere = nullptr;
	vertexResourceSphere->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSphere));


	Microsoft::WRL::ComPtr<ID3D12Resource> indexResourceSphere = CreateBufferResource(device.Get(), sizeof(uint32_t) * 1536);

	// IBV
	D3D12_INDEX_BUFFER_VIEW indexBufferViewSphere{};
	// リソースの先頭アドレスから使う
	indexBufferViewSphere.BufferLocation = indexResourceSphere->GetGPUVirtualAddress();
	// 使用するリソースのサイズはインデックス6つ分のサイズ
	indexBufferViewSphere.SizeInBytes = sizeof(uint32_t) * 1536;
	// インデックスはuint32_tとする
	indexBufferViewSphere.Format = DXGI_FORMAT_R32_UINT;

	uint32_t* indexDataSphere = nullptr;
	indexResourceSphere->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSphere));

	// Indexの計算
	for (uint32_t latIndex = 0; latIndex <= kSubdivision; latIndex++) {
		for (uint32_t lonIndex = 0; lonIndex <= kSubdivision; lonIndex++) {
			uint32_t baseIndex = (latIndex * (kSubdivision)+lonIndex) * 6;

			indexDataSphere[6 * (latIndex * kSubdivision + lonIndex)] = baseIndex;         // 0
			indexDataSphere[6 * (latIndex * kSubdivision + lonIndex) + 1] = baseIndex + 1; // 1
			indexDataSphere[6 * (latIndex * kSubdivision + lonIndex) + 2] = baseIndex + 2; // 2
			indexDataSphere[6 * (latIndex * kSubdivision + lonIndex) + 3] = baseIndex + 2; // 3
			indexDataSphere[6 * (latIndex * kSubdivision + lonIndex) + 4] = baseIndex + 1; // 4
			indexDataSphere[6 * (latIndex * kSubdivision + lonIndex) + 5] = baseIndex + 3; // 5
		}
	}


	// マテリアル用のリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = CreateBufferResource(device.Get(), sizeof(Material));
	// マテリアルにデータを書き込む
	Material* materialData = nullptr;
	// 書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	// 白を書きこむ
	materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData->enableLighting = false;
	materialData->uvTransform = MakeIdentity4x4();


	// マテリアル用のリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSphere = CreateBufferResource(device.Get(), sizeof(Material));
	// マテリアルにデータを書き込む
	Material* materialDataSphere = nullptr;
	// 書き込むためのアドレスを取得
	materialResourceSphere->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSphere));
	// 白を書きこむ
	materialDataSphere->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialDataSphere->enableLighting = true;
	materialDataSphere->uvTransform = MakeIdentity4x4();


	// Sprite用のマテリアルリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSprite = CreateBufferResource(device.Get(), sizeof(Material));
	// マテリアルにデータを読み込む
	Material* materialDataSprite = nullptr;
	// 書き込むためのアドレスを取得
	materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));
	// 白を書きこむ
	materialDataSprite->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialDataSprite->enableLighting = false;
	materialDataSprite->uvTransform = MakeIdentity4x4();

	Transform uvTransformSprite{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f},
	};

	// wvp用のリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResouce = CreateBufferResource(device.Get(), sizeof(TransformationMatrix));
	// データを書き込む
	TransformationMatrix* wvpData = nullptr;
	// 書き込むためのアドレスを取得
	wvpResouce->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	// 単位行列を書き込んでおく
	wvpData->WVP = MakeIdentity4x4();

	// Sprite用のTransformationMatrix用のリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceSprite = CreateBufferResource(device.Get(), sizeof(TransformationMatrix));
	// データを書き込む
	TransformationMatrix* transformationMatrixDataSprite = nullptr;
	// 書き込むためのアドレスを取得
	transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));
	// 単位行列を書き込んでおく
	transformationMatrixDataSprite->WVP = MakeIdentity4x4();


	// Sphereのwvp用のリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResouceSphere = CreateBufferResource(device.Get(), sizeof(TransformationMatrix));
	// データを書き込む
	TransformationMatrix* wvpDataSphere = nullptr;
	// 書き込むためのアドレスを取得
	wvpResouceSphere->Map(0, nullptr, reinterpret_cast<void**>(&wvpDataSphere));
	// 単位行列を書き込んでおく
	wvpDataSphere->WVP = MakeIdentity4x4();


	// Light用のマテリアルリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceLight = CreateBufferResource(device.Get(), sizeof(DirectionalLight));

	// 頂点バッファービューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewLight{};
	// リソース用の先頭のアドレスから使う
	vertexBufferViewLight.BufferLocation = materialResourceLight->GetGPUVirtualAddress();

	// 使用するリソースのサイズは頂点6つ分のサイズ
	vertexBufferViewLight.SizeInBytes = sizeof(DirectionalLight);
	// 1頂点当たりのサイズ
	vertexBufferViewLight.StrideInBytes = sizeof(DirectionalLight);

	DirectionalLight* directionalLightData = nullptr;

	materialResourceLight->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));

	directionalLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	directionalLightData->direction = { 0.0f,-1.0f,0.0f };
	directionalLightData->intensity = 1.0f;


	// ビューポート
	D3D12_VIEWPORT viewPort{};
	// クライアント領域のサイズと一緒にして画面全体に表示
	viewPort.Width = kWindowWidth;
	viewPort.Height = kWindowHeight;
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;

	// シザー矩形
	D3D12_RECT scissorRect{};
	// 基本的にビューポートと同じ矩形が構成されるようにする
	scissorRect.left = 0;
	scissorRect.right = kWindowWidth;
	scissorRect.top = 0;
	scissorRect.bottom = kWindowHeight;


	Transform transform{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };

	Transform transformSphere{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };

	Transform cameraTransform{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,-10.0f} };

	Transform transformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	// ImGuiの初期化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX12_Init(device.Get(),
		swapChainDesc.BufferCount,
		rtvDesc.Format,
		srvDescriptorHeap.Get(),
		srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());


	// Textureを読んで転送する
	DirectX::ScratchImage mipImages = LoadTexture("./Resources/uvChecker.png");
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = CreateTextureResource(device.Get(), metadata);
	UploadTextureData(textureResource.Get(), mipImages);

	// 2枚目のTextureを読んで転送する
	DirectX::ScratchImage mipImages2 = LoadTexture(modelData.material.textureFilePath);
	const DirectX::TexMetadata& metadata2 = mipImages2.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource2 = CreateTextureResource(device.Get(), metadata2);
	UploadTextureData(textureResource2.Get(), mipImages2);


	// metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	// 2つ目のmetaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
	srvDesc2.Format = metadata2.format;
	srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);


	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = GetCPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, 1);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = GetGPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, 1);

	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2 = GetCPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, 2);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2 = GetGPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, 2);

	// SRVの生成
	device->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);

	// SRVの生成
	device->CreateShaderResourceView(textureResource2.Get(), &srvDesc2, textureSrvHandleCPU2);



	// 球

	// 経度分割1つ分の角度φ
	const float kLonEvery = float(M_PI * 2.0f) / float(kSubdivision);
	// 緯度分割1つ分の角度θ
	const float kLatEvery = float(M_PI) / float(kSubdivision);
	// 緯度の方向に分割
	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = float(-M_PI / 2.0f) + kLatEvery * latIndex; // θ
		// 経度方向に分割しながら線を描く
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;
			float lon = lonIndex * kLonEvery; // φ
			// 頂点データを入力する。基準点a
			vertexDataSphere[start].position.x = std::cos(lat) * std::cos(lon);
			vertexDataSphere[start].position.y = std::sin(lat);
			vertexDataSphere[start].position.z = std::cos(lat) * std::sin(lon);
			vertexDataSphere[start].position.w = 1.0f;
			vertexDataSphere[start].texcoord.x = float(lonIndex) / float(kSubdivision);
			vertexDataSphere[start].texcoord.y = 1.0f - float(latIndex) / float(kSubdivision);
			vertexDataSphere[start].normal.x = vertexDataSphere[start].position.x;
			vertexDataSphere[start].normal.y = vertexDataSphere[start].position.y;
			vertexDataSphere[start].normal.z = vertexDataSphere[start].position.z;
			// b
			vertexDataSphere[start + 1].position.x = std::cos(lat + kLatEvery) * std::cos(lon);
			vertexDataSphere[start + 1].position.y = std::sin(lat + kLatEvery);
			vertexDataSphere[start + 1].position.z = std::cos(lat + kLatEvery) * std::sin(lon);
			vertexDataSphere[start + 1].position.w = 1.0f;
			vertexDataSphere[start + 1].texcoord.x = float(lonIndex) / float(kSubdivision);
			vertexDataSphere[start + 1].texcoord.y = 1.0f - float(latIndex + 1) / float(kSubdivision);
			vertexDataSphere[start + 1].normal.x = vertexDataSphere[start + 1].position.x;
			vertexDataSphere[start + 1].normal.y = vertexDataSphere[start + 1].position.y;
			vertexDataSphere[start + 1].normal.z = vertexDataSphere[start + 1].position.z;

			// c
			vertexDataSphere[start + 2].position.x = std::cos(lat) * std::cos(lon + kLonEvery);
			vertexDataSphere[start + 2].position.y = std::sin(lat);
			vertexDataSphere[start + 2].position.z = std::cos(lat) * std::sin(lon + kLonEvery);
			vertexDataSphere[start + 2].position.w = 1.0f;
			vertexDataSphere[start + 2].texcoord.x = float(lonIndex + 1) / float(kSubdivision);
			vertexDataSphere[start + 2].texcoord.y = 1.0f - float(latIndex) / float(kSubdivision);
			vertexDataSphere[start + 2].normal.x = vertexDataSphere[start + 2].position.x;
			vertexDataSphere[start + 2].normal.y = vertexDataSphere[start + 2].position.y;
			vertexDataSphere[start + 2].normal.z = vertexDataSphere[start + 2].position.z;

			// d
			vertexDataSphere[start + 3].position.x = std::cos(lat + kLatEvery) * std::cos(lon + kLonEvery);
			vertexDataSphere[start + 3].position.y = std::sin(lat + kLatEvery);
			vertexDataSphere[start + 3].position.z = std::cos(lat + kLatEvery) * std::sin(lon + kLonEvery);
			vertexDataSphere[start + 3].position.w = 1.0f;
			vertexDataSphere[start + 3].texcoord.x = float(lonIndex + 1) / float(kSubdivision);
			vertexDataSphere[start + 3].texcoord.y = 1.0f - float(latIndex + 1) / float(kSubdivision);
			vertexDataSphere[start + 3].normal.x = vertexDataSphere[start + 3].position.x;
			vertexDataSphere[start + 3].normal.y = vertexDataSphere[start + 3].position.y;
			vertexDataSphere[start + 3].normal.z = vertexDataSphere[start + 3].position.z;

			// c
			/*vertexDataSphere[start + 3].position.x = std::cos(lat) * std::cos(lon + kLonEvery);
			vertexDataSphere[start + 3].position.y = std::sin(lat);
			vertexDataSphere[start + 3].position.z = std::cos(lat) * std::sin(lon + kLonEvery);
			vertexDataSphere[start + 3].position.w = 1.0f;
			vertexDataSphere[start + 3].texcoord.x = float(lonIndex + 1) / float(kSubdivision);
			vertexDataSphere[start + 3].texcoord.y = 1.0f - float(latIndex) / float(kSubdivision);
			vertexDataSphere[start + 3].normal.x = vertexDataSphere[start + 3].position.x;
			vertexDataSphere[start + 3].normal.y = vertexDataSphere[start + 3].position.y;
			vertexDataSphere[start + 3].normal.z = vertexDataSphere[start + 3].position.z;*/
			// b
			//vertexDataSphere[start + 4].position.x = std::cos(lat + kLatEvery) * std::cos(lon);
			//vertexDataSphere[start + 4].position.y = std::sin(lat + kLatEvery);
			//vertexDataSphere[start + 4].position.z = std::cos(lat + kLatEvery) * std::sin(lon);
			//vertexDataSphere[start + 4].position.w = 1.0f;
			//vertexDataSphere[start + 4].texcoord.x = float(lonIndex) / float(kSubdivision);
			//vertexDataSphere[start + 4].texcoord.y = 1.0f - float(latIndex + 1) / float(kSubdivision);
			//vertexDataSphere[start + 4].normal.x = vertexDataSphere[start + 4].position.x;
			//vertexDataSphere[start + 4].normal.y = vertexDataSphere[start + 4].position.y;
			//vertexDataSphere[start + 4].normal.z = vertexDataSphere[start + 4].position.z;

		}
	}


	bool useMonsterBall = true;

	/*System::Initialize(kWindowTitle, 1280, 720);*/

	MSG msg{};
	// ウインドウのxボタンが押されるまでループ
	while (msg.message != WM_QUIT) {
		//Windowにメッセージが来てたら最優先で処理させる
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
			// ゲームの処理

			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			// 開発用UIの処理
			
			ImGui::DragFloat3("cameraTranslation", &cameraTransform.translate.x, 0.01f);

			ImGui::SliderAngle("CameraRotateX", &cameraTransform.rotate.x);
			ImGui::SliderAngle("CameraRotateY", &cameraTransform.rotate.y);
			ImGui::SliderAngle("CameraRotateZ", &cameraTransform.rotate.z);

			ImGui::SliderAngle("RotateX", &transform.rotate.x);
			ImGui::SliderAngle("RotateY", &transform.rotate.y);
			ImGui::SliderAngle("RotateZ", &transform.rotate.z);
			ImGui::ColorEdit4("Color", (float*)&materialData->color);

			ImGui::Checkbox("useMonsterBall", &useMonsterBall);

			ImGui::Checkbox("useLight", &materialData->enableLighting);
			ImGui::SliderFloat3("LightDirector", &directionalLightData->direction.x, -1.0f, 1.0f);
			ImGui::ColorEdit4("LightColor", (float*)&directionalLightData->color);
			ImGui::DragFloat("intencity", &directionalLightData->intensity, 0.01f);

			ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
			ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
			ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);


			// 指定した深度で画面全体をクリアする
			commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);


			UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();

			// TransitionBarrierの設定
			D3D12_RESOURCE_BARRIER barrier{};

			//TransitionBarrierの設定
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

			// Noneにしておく
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

			// バリアを張る対象のリソース
			barrier.Transition.pResource = swapChainResources[backBufferIndex].Get();

			// 遷移前のResourceState
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;

			// 遷移後のResourceState
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

			// TransitionBarrierを張る
			commandList->ResourceBarrier(1, &barrier);


			// 描画先のRTVを設定する
			commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, nullptr);

			// 描画先のRTVとDSVを設定する
			commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, &dsvHandle);

			// 指定した色で画面全体をクリアする
			float clearColor[] = { 0.1f,0.25f,0.5f,1.0f }; // 
			commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);

			// 描画用のDescriptorHeapの設定
			ID3D12DescriptorHeap* descriptorHeaps[] = { srvDescriptorHeap.Get() };
			commandList->SetDescriptorHeaps(1, descriptorHeaps);

			// ImGuiの内部コマンドを生成する
			ImGui::Render();

			transformSphere.rotate.y += 0.01f;


			// 
			Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
			Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
			Matrix4x4 viewMatrix = Inverse4x4(cameraMatrix);
			Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(kWindowWidth) / float(kWindowHeight), 0.1f, 100.0f);
			Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
			wvpData->WVP = worldViewProjectionMatrix;
			wvpData->World = worldMatrix;


			// Sphere用のWorldViewProjectionMatrixを作る
			Matrix4x4 worldMatrixSphere = MakeAffineMatrix(transformSphere.scale, transformSphere.rotate, transformSphere.translate);
			Matrix4x4 cameraMatrixSphere = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
			Matrix4x4 viewMatrixSphere = Inverse4x4(cameraMatrixSphere);
			Matrix4x4 projectionMatrixSphere = MakePerspectiveFovMatrix(0.45f, float(kWindowWidth) / float(kWindowHeight), 0.1f, 100.0f);
			Matrix4x4 worldViewProjectionMatrixSphere = Multiply(worldMatrixSphere, Multiply(viewMatrixSphere, projectionMatrixSphere));
			wvpDataSphere->WVP = worldViewProjectionMatrixSphere;
			wvpDataSphere->World = worldMatrixSphere;


			// Sprite用のWorldViewProjectionMatrixを作る
			Matrix4x4 worldMatrixSprite = MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
			Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
			Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, float(kWindowWidth), float(kWindowHeight), 0.0f, 100.0f);
			Matrix4x4 worldViewProjectionMatrixSprite = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));
			transformationMatrixDataSprite->WVP = worldViewProjectionMatrixSprite;
			transformationMatrixDataSprite->World = worldMatrixSprite;

			Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite.scale);
			uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite.rotate.z));
			uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite.translate));
			materialDataSprite->uvTransform = uvTransformMatrix;

			/*/////////////////////////////////////////////////////////////////////////////
					いざ描画
			*//////////////////////////////////////////////////////////////////////////////
			commandList->RSSetViewports(1, &viewPort); // Viewportを設定
			commandList->RSSetScissorRects(1, &scissorRect); // Scissorを設定

			// RootSignatureを設定
			commandList->SetGraphicsRootSignature(rootSignature.Get());
			commandList->SetPipelineState(graphicsPipelineState.Get()); // PSOを設定
			commandList->IASetVertexBuffers(0, 1, &vertexBufferViewSphere); // VBVを設定
			commandList->IASetIndexBuffer(&indexBufferViewSphere); // IBVを設定
			//commandList->IASetVertexBuffers(1, 1, &vertexBufferViewLight); // VBVを設定

			// 形状を設定
			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			// マテリアルCBufferの場所を設定
			commandList->SetGraphicsRootConstantBufferView(0, materialResourceSphere->GetGPUVirtualAddress());
			// wvp用のCBufferの場所を設定
			commandList->SetGraphicsRootConstantBufferView(1, wvpResouce->GetGPUVirtualAddress());
			// 
			commandList->SetGraphicsRootConstantBufferView(3, materialResourceLight->GetGPUVirtualAddress());
			// SRVのDescriptorTableの先頭を設定
			commandList->SetGraphicsRootDescriptorTable(2, useMonsterBall ? textureSrvHandleGPU2 : textureSrvHandleGPU);

			// Sphereの描画
			//commandList->DrawIndexedInstanced(1536, 1, 0, 0, 0);


			// マテリアルCBufferの場所を設定
			commandList->SetGraphicsRootConstantBufferView(0, materialResourceSprite->GetGPUVirtualAddress());
			// 
			commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
			// Spriteの描画
			commandList->IASetVertexBuffers(0, 1, &vertexBufferViewSprite); // VBVを設定
			// 
			commandList->IASetIndexBuffer(&indexBufferViewSprite); // IBVを設定
			// TransformationMatrixCBufferの場所を設定
			commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSprite->GetGPUVirtualAddress());
			// Spriteの描画
			//commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);


			commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
			// TransformationMatrixCBufferの場所を設定
			commandList->SetGraphicsRootConstantBufferView(1, wvpResouce->GetGPUVirtualAddress());
			commandList->IASetVertexBuffers(0, 1, &vertexBufferView); // VBVを設定

			// Modelの描画
			commandList->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);


			// 実際のCommandListのImGuiの描画コマンドを積む
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());

			// 画面に描く処理はすべて終わり、画面に移すので状態を遷移
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

			// TransitionBarrierを張る
			commandList->ResourceBarrier(1, &barrier);

			// コマンドリストの内容を確定させる。
			hr = commandList->Close();
			assert(SUCCEEDED(hr));

			/*/////////////////////////////////////////////////////////////////////////////
				コマンドをキック
			*//////////////////////////////////////////////////////////////////////////////
			// GPUにコマンドリストの実行を行わせる
			ID3D12CommandList* commandLists[] = { commandList.Get() };
			commandQueue->ExecuteCommandLists(1, commandLists);

			// GPUとOSに画面の交換を行うよう通知する
			swapChain->Present(1, 0);

			// Fenceの値を更新
			fenceValue++;

			//GPUがここまでたどり着いたときに、Fenceの値を指定した値に代入するようにSignalを送る
			commandQueue->Signal(fence.Get(), fenceValue);

			// Fenceの値が指定したSignalにたどり着いているか確認する
			if (fence->GetCompletedValue() < fenceValue) {
				// 指定したSignalにたどり着いていないので、たどり着くまで待つようにイベントを設定する
				fence->SetEventOnCompletion(fenceValue, fenceEvent);

				// イベントを待つ
				WaitForSingleObject(fenceEvent, INFINITE);
			}

			// 次のフレーム用のコマンドリストを準備
			hr = commandAllocator->Reset();
			assert(SUCCEEDED(hr));
			hr = commandList->Reset(commandAllocator.Get(), nullptr);
			assert(SUCCEEDED(hr));

		}
	}

	// ImGuiの終了処理。初期化と逆順
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	/*/////////////////////////////////////////////////////////////////////////////
			解放
	*//////////////////////////////////////////////////////////////////////////////
	CloseHandle(fenceEvent);

	CloseWindow(hwnd);

	// COMの終了処理
	CoUninitialize();


	return 0;
}