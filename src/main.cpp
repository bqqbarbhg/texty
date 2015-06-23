#define UNICODE
#include <Windows.h>
#include <string.h>

wchar_t current_filename[MAX_PATH];
char text_buffer[10*1024*1024] = "";
int cursor;
HFONT font = NULL;

struct Line {
	char *start;
	char *end;
};

char *find_line_end(char *c) {
	while (*c != '\0' && *c != '\n')
		c++;
	return c;
}

Line next_line(Line line) {
	if (*line.end == '\0') {
		Line null_line = { 0 };
		return null_line;
	}
	line.start = line.end + 1;
	line.end = find_line_end(line.start);
	return line;
}

Line find_line(int row) {
	Line line = { text_buffer, find_line_end(text_buffer) };

	for (int i = 0; i < row; i++) {
		line = next_line(line);
	}

	return line;
}

bool valid_line(Line line) {
	return line.start && line.end;
}

void paint_window(HWND window)
{
	HBRUSH clear_brush = CreateSolidBrush(RGB(30, 30, 30));
	HBRUSH cursor_brush = CreateSolidBrush(RGB(255, 0, 0));

	if (font == NULL) {
		font = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH|FF_MODERN,
			TEXT("Consolas"));
	}

	PAINTSTRUCT paint;
	HDC dc = BeginPaint(window, &paint);
	RECT area = paint.rcPaint;

	SelectObject(dc, font);

	FillRect(dc, &area, clear_brush);

	int cx = -1, cy = -1;

	int y = 2;
	for (Line line = find_line(0); valid_line(line); line = next_line(line)) {

		// No need to draw the text above the area
		if (y + 14 < area.top) {
			y += 14;
			continue;
		}

		int x = 2;
		for (char *p = line.start; p != line.end; p++) {

			if (text_buffer + cursor == p) {
				cx = x;
				cy = y;
			}

			char c = *p;
			SIZE size;
			char buf[2] = { c, 0 };
			GetTextExtentPoint32A(dc, buf, 1, &size);
			TextOutA(dc, x, y, buf, 1);
			x += size.cx;
		}

		if (text_buffer + cursor == line.end) {
			cx = x;
			cy = y;
		}

		y += 14;

		// No need to draw the text below the area
		if (y > area.bottom)
			break;
	}

	RECT cursor_rect = { cx, cy, cx + 4, cy + 14 };
	if (cx >= 0)
		FillRect(dc, &cursor_rect, cursor_brush);

	EndPaint(window, &paint);

	DeleteObject(clear_brush);
}

LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {

	case WM_PAINT:
		paint_window(hwnd);
		break;

	case WM_CHAR:

		if (wParam != VK_RETURN && wParam != VK_BACK && wParam != VK_LEFT && wParam != VK_RIGHT) {
			memmove(text_buffer + cursor + 1, text_buffer + cursor, strlen(text_buffer + cursor) + 1);
			text_buffer[cursor++] = (char)wParam;
			RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
		}

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
		} else if (wParam == VK_F5) {
			HANDLE file = CreateFileW(current_filename,
					GENERIC_WRITE,
					FILE_SHARE_WRITE,
					NULL,
					OPEN_ALWAYS,
					0,
					NULL);

			if (file != INVALID_HANDLE_VALUE) {
				DWORD num_written;
				WriteFile(file, text_buffer, (DWORD)strlen(text_buffer), &num_written, NULL);
				CloseHandle(file);
			}
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
	wnd_class.lpszClassName = TEXT("TextyEdit");

	int argc;
	LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);

	if (argc > 1) {

		wcscpy(current_filename, argv[1]);

		HANDLE file = CreateFileW(argv[1],
				GENERIC_READ,
				FILE_SHARE_READ,
				NULL,
				OPEN_EXISTING,
				0,
				NULL);

		if (file != INVALID_HANDLE_VALUE) {

			DWORD num_read;
			ReadFile(file, text_buffer, sizeof(text_buffer) - 1, &num_read, NULL);
			text_buffer[num_read] = 0;

			CloseHandle(file);
		}
	}

	LocalFree(argv);

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

	HWND window = CreateWindowExW(ex_style, MAKEINTATOM(class_atom), TEXT("Texty"), style,
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

