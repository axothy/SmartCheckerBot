#include <windows.h>
#include "player.h"
#include "SmartBot.h"
#include <gl/gl.h>
#include <iostream>

#pragma comment(lib, "opengl32.lib")


LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);

//Ф-я возвращает координату твоего клика по карте
void ScreenToOpenGL(HWND hwnd, int x, int y, float* ox, float* oy) {
	RECT rct;
	GetClientRect(hwnd, &rct);
	*ox = x / (float)rct.right * playboard.getW();
	*oy = playboard.getH() - y / (float)rct.bottom * playboard.getH();
}

//Проверка кликнули ли мы на доску или вне границ карты
bool IsCellInMap(int x, int y) {
	return (x >= 0) && (y >= 0) && (x <= playboard.getW()) && (y <= playboard.getH());
}

Player* player1;
Player* player2;

void Menu() {

	int play_or_watch = 0;
	std::cout << "Who's playing white?" << std::endl;
	std::cout << "EASY BOT - PRESS 1" << std::endl;
	std::cout << "MEDIUM BOT - PRESS 2" << std::endl;
	std::cout << "SMART BOT BOT - PRESS 3" << std::endl;
	std::cout << "REAL PLAYER - PRESS 4" << std::endl;

	while ((play_or_watch == 1 || play_or_watch == 2 || play_or_watch == 3 || play_or_watch == 4) != true) {
		std::cin >> play_or_watch;
	}
	if (play_or_watch == 1) {
		player1 = new EasyBot(&playboard);
	}
	else if (play_or_watch == 2) {
		player1 = new MediumBot(&playboard);
	}
	else if (play_or_watch == 3) {
		player1 = new SmartBot(2, &playboard);
	}
	else if (play_or_watch == 4) {
		player1 = new Player(&playboard);
	}

	std::cout << "Who's playing black?" << std::endl;
	std::cout << "EASY BOT - PRESS 1" << std::endl;
	std::cout << "MEDIUM BOT - PRESS 2" << std::endl;
	std::cout << "SMART BOT BOT - PRESS 3" << std::endl;
	std::cout << "REAL PLAYER - PRESS 4" << std::endl;

	play_or_watch = 0;
	while ((play_or_watch == 1 || play_or_watch == 2 || play_or_watch == 3 || play_or_watch == 4) != true) {
		std::cin >> play_or_watch;
	}
	if (play_or_watch == 1) {
		player2 = new EasyBot(&playboard);
	}
	else if (play_or_watch == 2) {
		player2 = new MediumBot(&playboard);
	}
	else if (play_or_watch == 3) {
		player2 = new SmartBot(3, &playboard);
	}
	else if (play_or_watch == 4) {
		player2 = new Player(&playboard);
	}
}


int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	WNDCLASSEX wcex;
	HWND hwnd;
	HDC hDC;
	HGLRC hRC;
	MSG msg;
	BOOL bQuit = FALSE;
	float theta = 0.0f;


	/* register windsow class */
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_OWNDC;
	wcex.lpfnWndProc = WindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = (LPCWSTR)"GLSample";
	wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;

	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);

	if (!RegisterClassEx(&wcex))
		return 0;

	/* create main window */
	hwnd = CreateWindowEx(0,
		(LPCWSTR)"GLSample",
		TEXT("Русские шашки"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		512,
		512,
		NULL,
		NULL,
		hInstance,
		NULL);

	ShowWindow(hwnd, nCmdShow);

	/* enable OpenGL for the window */
	EnableOpenGL(hwnd, &hDC, &hRC);

	glScalef(float(512) / float(512), 1, 1); //функция масштабирования экрана,
	// растягивает сжимает или отражает по осям х у z

	AllocConsole(); // открываем консоль, куда будем записывать счет, ход игры и прочее через cout
	freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
	freopen_s((FILE**)stdin, "CONIN$", "r", stdin);
	std::cout << "GAME RUSSIAN CHECKERS" << std::endl;

	Menu();

	/* program main loop */
	while (!bQuit)
	{
		/* check for messages */
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			/* handle or dispatch messages */
			if (msg.message == WM_QUIT)
			{
				bQuit = TRUE;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			/* OpenGL animation code goes here */
			glClearColor(1.0f, 1.0f, 1.0f, 0.5f);
			glClear(GL_COLOR_BUFFER_BIT);

			playboard.drawPlayboard();

			SwapBuffers(hDC);
			Sleep(200);
		}
	}

	/* shutdown OpenGL */
	DisableOpenGL(hwnd, hDC, hRC);

	/* destroy the window explicitly */
	DestroyWindow(hwnd);

	return int(msg.wParam);
}


int turn = 0;


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		break;

	case WM_DESTROY:
		return 0;

	case WM_LBUTTONDOWN:
	{
		POINTFLOAT pf;
		ScreenToOpenGL(hwnd, LOWORD(lParam), HIWORD(lParam), &pf.x, &pf.y);
		int x = int(pf.x);
		int y = int(pf.y);
		int move = 0;
		if (IsCellInMap(x, y)) {
			if (turn % 2 == 0) {
				if (player1->Turn(x, y, WHITE)) {
					turn++;
				}
			}
			else {
				if (player2->Turn(x, y, BLACK)) {
					turn++;
				}
			}
		}
	}
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		}
	}
	break;

	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	return 0;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
	PIXELFORMATDESCRIPTOR pfd;

	int iFormat;

	/* get the device context (DC) */
	*hDC = GetDC(hwnd);

	/* set the pixel format for the DC */
	ZeroMemory(&pfd, sizeof(pfd));

	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;

	iFormat = ChoosePixelFormat(*hDC, &pfd);

	SetPixelFormat(*hDC, iFormat, &pfd);

	/* create and enable the render context (RC) */
	*hRC = wglCreateContext(*hDC);

	wglMakeCurrent(*hDC, *hRC);
}

void DisableOpenGL(HWND hwnd, HDC hDC, HGLRC hRC)
{
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hRC);
	ReleaseDC(hwnd, hDC);
}