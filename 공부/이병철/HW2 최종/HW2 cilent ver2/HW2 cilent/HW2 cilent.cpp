// HW1.cpp: 응용 프로그램의 진입점을 정의합니다.
//
#pragma warning(disable : 4996)

#include "stdafx.h"
#include "HW2 cilent.h"

#define MAX_LOADSTRING 100

SOCKET sock; // 소켓
char IPAdd[30] = "127.0.0.1";

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

void err_quit(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCWSTR)msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}
void err_display(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", (LPCSTR)msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}
int recvn(SOCKET s, char *buf, int len, int flags)
{
	int received;
	char *ptr = buf;
	int left = len;

	while (left > 0) {
		received = recv(s, ptr, left, flags);
		if (received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (received == 0)
			break;
		left -= received;
		ptr += received;
	}

	return (len - left);
}
DWORD WINAPI ClientMain(LPVOID arg);

CImage character1;
CImage character2;
POINT characterPos1 = { 400,0 }; //캐릭터 초기 위치 값
POINT characterPos2 = { 400,700 };
BOOL KeyBuffer[256];

int retval;

#pragma pack(1)
struct Client_Send_Struct {
	BOOL KeyBuffer[256];
};
#pragma pack()

#pragma pack(1)
struct Client_Recv_Struct {
	POINT p1;
};
#pragma pack()

Client_Send_Struct CSS;
Client_Recv_Struct CRS;

								  // 이 코드 모듈에 들어 있는 함수의 정방향 선언입니다.
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: 여기에 코드를 입력합니다.

	printf("IP INPUT : ");
	scanf_s("%s", IPAdd, sizeof(IPAdd));

	HANDLE hThread;

	hThread = CreateThread(NULL, 0, ClientMain, NULL, 0, NULL);

	// 전역 문자열을 초기화합니다.
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_HW2CILENT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	
	// 응용 프로그램 초기화를 수행합니다.
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_HW2CILENT));

	MSG msg;

	// 기본 메시지 루프입니다.
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}

DWORD WINAPI ClientMain(LPVOID arg)
{

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;
	// socket()
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit((char*)"socket()");

	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(IPAdd);
	//serveraddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit((char*)"connect()");

	while (1) 
	{
		// recv 부분
		retval = recvn(sock, (char *)&CRS, sizeof(CRS), 0);
		if (retval == SOCKET_ERROR)
		{
			err_display((char*)"recv(22)");
			exit(1);
		}

		memcpy(&characterPos1, &CRS.p1, sizeof(characterPos1));
	}

	// closesocket()
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
}


//
//  함수: MyRegisterClass()
//
//  목적: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HW2CILENT));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	//wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_HW1);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   목적: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   설명:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

	RECT rcWindow = { 0, 0, 800, 800 };

	AdjustWindowRect(&rcWindow, WS_OVERLAPPEDWINDOW, false);
	HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		0, 0, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  목적:  주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 응용 프로그램 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	DWORD black = RGB(0, 0, 0);
	DWORD white = RGB(255, 255, 255);

	HBRUSH BackBrush = CreateSolidBrush(RGB(255, 255, 255));

	HBRUSH BlackBrush = CreateSolidBrush(black);
	HBRUSH WhiteBrush = CreateSolidBrush(white);

	static RECT rectView;

	RECT backGround{ 0, 0, BOARDSIZE, BOARDSIZE };

	switch (message)
	{
	case WM_CREATE:
		GetClientRect(hWnd, &rectView);
		character1.Load(TEXT("no1.png"));

		break;
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// 메뉴 선택을 구문 분석합니다.
		switch (wmId)
		{
		case IDM_ABOUT:
			//DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_LEFT:
		{
			KeyBuffer[VK_LEFT] = true;
			memcpy(&CSS.KeyBuffer, &KeyBuffer, sizeof(KeyBuffer));
			// 고정길이 전송
			retval = send(sock, (char *)&CSS, sizeof(CSS), 0);
			if (retval == SOCKET_ERROR)
			{
				err_display((char*)"send()");
				return 0;
			}
		}
		break;
		case VK_RIGHT: 
		{
			KeyBuffer[VK_RIGHT] = true;
			memcpy(&CSS.KeyBuffer, &KeyBuffer, sizeof(KeyBuffer));
			// 고정길이 전송
			retval = send(sock, (char *)&CSS, sizeof(CSS), 0);
			if (retval == SOCKET_ERROR)
			{
				err_display((char*)"send()");
				return 0;
			}
		}
					   break;
		case VK_DOWN:
		{
			KeyBuffer[VK_DOWN] = true;
			memcpy(&CSS.KeyBuffer, &KeyBuffer, sizeof(KeyBuffer));
			// 고정길이 전송
			retval = send(sock, (char *)&CSS, sizeof(CSS), 0);
			if (retval == SOCKET_ERROR)
			{
				err_display((char*)"send()");
				return 0;
			}
		}
					  break;
		case VK_UP: 
		{

			KeyBuffer[VK_UP] = true;
			memcpy(&CSS.KeyBuffer, &KeyBuffer, sizeof(KeyBuffer));
			// 고정길이 전송
			retval = send(sock, (char *)&CSS, sizeof(CSS), 0);
			if (retval == SOCKET_ERROR)
			{
				err_display((char*)"send()");
				return 0;
			}
		}
					break;
		case VK_SPACE:
			break;
		case VK_ESCAPE:
			exit(1);
		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_KEYUP:
		KeyBuffer[wParam] = FALSE;
		//InvalidateRect(hWnd, NULL, TRUE);
		break;


	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다.

		CImage img;
		img.Create(rectView.right, rectView.bottom, 24);

		HDC memDC = img.GetDC();
		RECT rc;
		FillRect(memDC, &backGround, BackBrush);

		for (int i = 0; i < BOARDNUM; ++i) {
			for (int j = 0; j < BOARDNUM; ++j) {

				rc.left = j * (BOARDSIZE / BOARDNUM);
				rc.top = i * (BOARDSIZE / BOARDNUM);

				rc.right = (j + 1)*(BOARDSIZE / BOARDNUM);
				rc.bottom = (i + 1)*(BOARDSIZE / BOARDNUM);

				if ((i + j) % 2 == 0)
					FillRect(memDC, &rc, BlackBrush);

			}
		}
		//보정 값 25
		character1.Draw(memDC, 0 + characterPos1.x + 25, 0 + characterPos1.y + 25, CHARACTERSIZE, CHARACTERSIZE);
		img.Draw(hdc, 0, 0);
		img.ReleaseDC();
		//InvalidateRect(hWnd, NULL, TRUE);
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


