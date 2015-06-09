#include <Windows.h>

LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASS wnd_class = { 0 };
	wnd_class.lpfnWndProc = window_proc;
	wnd_class.hInstance = hInstance;
	wnd_class.lpszClassName = "TextyEdit";

	ATOM class_atom = RegisterClass(&wnd_class);
	if (!class_atom) {
		return 1;
	}

	DWORD ex_style = 0;

	DWORD style
		= WS_VISIBLE // Show the window on startup
		| WS_CAPTION // Has title bar
		| WS_SYSMENU // Close button
		| WS_MAXIMIZEBOX // Maximize button
		| WS_MINIMIZEBOX // Minimize button
		| WS_THICKFRAME // Resizable window
		;

	HWND window = CreateWindowEx(ex_style, MAKEINTATOM(class_atom), "Texty", style,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			NULL, NULL, hInstance, NULL);

	// Message loop
	for (;;) {

		MSG msg;
		BOOL ret = GetMessage(&msg, window, 0, 0);

		if (ret == -1) {
			// Some error, just crash for now
			return 1;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if (ret == 0) {
			// GetMessage returns 0 when the message is WM_QUIT, so after giving it to window let's
			// stop looping
			break;
		}
	}

	return 0;
}

