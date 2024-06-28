//#include "System.h"
//
//#include "DirectXCommon.h"
//#include "WinApp.h"
//#include <cstdint>
//#include <string>
//#include <format>
//#include <cassert>
//
//#include <d3d12.h>
//#include <dxgi1_6.h>
//
//#pragma comment(lib,"d3d12.lib")
//#pragma comment(lib,"dxgi.lib")
//
//std::wstring ConvertString(const std::string& str) {
//	if (str.empty()) {
//		return std::wstring();
//	}
//
//	auto sizeNeeded = MultiByteToWideChar(
//		CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), NULL, 0);
//	if (sizeNeeded == 0) {
//		return std::wstring();
//	}
//	std::wstring result(sizeNeeded, 0);
//	MultiByteToWideChar(
//		CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()),
//		&result[0], sizeNeeded);
//	return result;
//}
//
//std::string ConvertString(const std::wstring& str) {
//	if (str.empty()) {
//		return std::string();
//	}
//
//	auto sizeNeeded = WideCharToMultiByte(
//		CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0, NULL, NULL);
//	if (sizeNeeded == 0) {
//		return std::string();
//	}
//	std::string result(sizeNeeded, 0);
//	WideCharToMultiByte(
//		CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded, NULL,
//		NULL);
//	return result;
//}
//
//// WindowsAPI
//WinApp* winApp_ = nullptr;
//// DirectX
//DirectXCommon* dxCommon_ = nullptr;
//
//void System::Initialize(const char* title, int width, int height) {
//
//	winApp_ = WinApp::GetInstance();
//
//	// ゲームウインドウの作成
//	std::string titleWithVersion = std::string(title);
//	auto&& titleString = ConvertString(titleWithVersion);
//	winApp_->CreateGameWindow(
//		titleString.c_str(), WS_OVERLAPPEDWINDOW & ~(WS_MAXIMIZEBOX | WS_THICKFRAME), width,
//		height);
//
//	dxCommon_ = DirectXCommon::GetInstance();
//	dxCommon_->Initialize(winApp_);
//}
