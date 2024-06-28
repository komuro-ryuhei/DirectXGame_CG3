//#pragma once
//#include <Windows.h>
//#include <cstdint>
//
///// <summary>
///// ウインドウズアプリケーション
///// </summary>
//class WinApp {
//public: // 静的メンバ変数
//	// ウインドウサイズ
//	static const int32_t kWindowWidth = 1280;
//	static const int32_t kWindowHeight = 720;
//	// ウインドウクラス名
//	static const wchar_t kWindowClassName[];
//
//public: // 静的メンバ関数
//	/// <summary>
//	/// シングルトンインスタンスの取得
//	/// </summary>
//	/// <returns>シングルトンインスタンス</returns>
//	static WinApp* GetInstance();
//
//	/// <summary>
//	/// ウインドウプロシージャ
//	/// </summary>
//	/// <param name="hwnd">ウインドウハンドル</param>
//	/// <param name="msg">メッセージ番号</param>
//	/// <param name="wparam">メッセージ情報1</param>
//	/// <param name="lpalam">メッセージ情報2</param>
//	/// <returns></returns>
//	static LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
//
//public: // メンバ関数
//
//	/// <summary>
//	/// ゲームウインドウの作成
//	/// </summary>
//	/// <param name="title">ウインドウタイトル</param>
//	/// <param name="windowStyle">ウインドウの初期スタイル</param>
//	/// <param name="clientWidth">ウインドウのクライアントの横幅</param>
//	/// <param name="clientHeight">ウインドウのクライアントの縦幅</param>
//	void CreateGameWindow(const wchar_t* title = L"DirectXGame", UINT windowStyle = WS_OVERLAPPEDWINDOW,
//		int32_t clientWidth = kWindowWidth, int32_t clientHeight = kWindowHeight);
//
//	/// <summary>
//	/// ゲームウィンドウの破棄
//	/// </summary>
//	void TerminateGameWindow();
//
//	HWND GetHwnd() const { return hwnd_; }
//
//	HINSTANCE GetHInstance() const { return wndClass_.hInstance; }
//
//private: // メンバ変数
//
//	HWND hwnd_; // ウィンドウハンドル
//	WNDCLASSEX wndClass_{}; // ウィンドウクラス
//	UINT windowStyle_;
//	float aspectRatio_;
//};
//
