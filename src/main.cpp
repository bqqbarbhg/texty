#include <Windows.h>
#include <string.h>

char text_buffer[128] = "";
int cursor;

void paint_window(HWND window)
{
	HBRUSH clear_brush = CreateSolidBrush(RGB(30, 30, 30));
	HBRUSH cursor_brush = CreateSolidBrush(RGB(255, 0, 0));

	PAINTSTRUCT paint;
	HDC dc = BeginPaint(window, &paint);
	RECT area = paint.rcPaint;

	FillRect(dc, &area, clear_brush);
	size_t len = strlen(text_buffer);
	int x = 2;
	int y = 2;
	int cx = 2, cy = 2;
	for (size_t i = 0; i < len; i++) {
		char c = text_buffer[i];
		if (c == '\n') {
			x = 2;
			y += 14;
		}
		RECT rc = { 0, 0, 0, 0 };
		char buf[2] = { c, 0 };
		DrawText(dc, buf, 1, &rc, DT_CALCRECT);
		TextOut(dc, x, y, buf, 1);
		x += rc.right;
		if (i + 1 == cursor) {
			cx = x;
			cy = y;
		}
	}

	RECT cursor_rect = { cx, cy, cx + 4, cy + 14 };
	FillRect(dc, &cursor_rect,	cursor_brush);

	EndPaint(window, &paint);

	DeleteObject(clear_brush);
}

LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {

	case WM_PAINT:
		paint_window(hwnd);
		break;

	case WM_KEYDOWN:
		if (wParam == VK_RETURN) {
			memmove(text_buffer + cursor + 1, text_buffer + cursor, strlen(text_buffer + cursor) + 1);
			text_buffer[cursor++] = '\n';
		} else if (wParam == VK_BACK) {
			memmove(text_buffer + cursor - 1, text_buffer + cursor, strlen(text_buffer + cursor) + 1);
			cursor--;
		} else if (wParam == VK_LEFT) {
			cursor--;
		} else if (wParam == VK_RIGHT) {
			cursor++;
		} else {
			memmove(text_buffer + cursor + 1, text_buffer + cursor, strlen(text_buffer + cursor) + 1);
			text_buffer[cursor++] = (char)wParam;
		}
		RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
		break;

	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
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

