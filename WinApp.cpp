//#include "WinApp.h"
//
//#include <string>
//
//const wchar_t WinApp::kWindowClassName[] = L"CG2WindowClass";
//
//WinApp* WinApp::GetInstance() {
//	static WinApp instance;
//	return &instance;
//}
//
//// ウインドウプロシージャ
//LRESULT WinApp::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
//
//	// メッセージに応じてゲーム固有の処理
//	switch (msg) {
//		// ウインドウが破棄された
//	case WM_DESTROY:
//		// OSに対してアプリの終了を伝える
//		PostQuitMessage(0);
//		return 0;
//	}
//
//	// 標準のメッセージ処理を行う
//	return DefWindowProc(hwnd, msg, wparam, lparam);
//}
//
//void WinApp::CreateGameWindow(const wchar_t* title, UINT windowStyle,int32_t clientWidth, int32_t clientHeight) {
//
//	windowStyle_ = windowStyle;
//	aspectRatio_ = float(clientWidth) / float(clientHeight);
//
//	/*WNDCLASS wc{};
//	 ウインドウプロシージャ
//	wc.lpfnWndProc = WindowProc;
//	 ウインドウクラス名
//	wc.lpszClassName = kWindowClassName;
//	 インスタンスハンドル
//	wc.hInstance = GetModuleHandle(nullptr);
//	カーソル
//	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
//
//	 ウインドウクラスを登録する
//	RegisterClass(&wc);
//
//	 ウインドウサイズを表す構造体にクライアント領域を入れる
//	RECT wrc = { 0,0,clientWidth,clientHeight };
//
//	 クライアント領域を元に実際のサイズにwrcを変更してもらう
//	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);
//
//	 ウインドの生成
//	hwnd_ = CreateWindow(
//		wc.lpszClassName,
//		title,
//		WS_OVERLAPPEDWINDOW,
//		CW_USEDEFAULT,
//		CW_USEDEFAULT,
//		wrc.right - wrc.left,
//		wrc.bottom - wrc.top,
//		nullptr,
//		nullptr,
//		wc.hInstance,
//		nullptr
//	);
//
//	 ウインドウを表示する
//	ShowWindow(hwnd_, SW_SHOW);*/
//}
