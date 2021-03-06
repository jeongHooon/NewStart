// HW2 server.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"

#define SERVER_PORT 9000
#define BUFSIZE    30000
#define MAX_BUFFER        1024
#define MAX_PLAYER 10

char IPAdd[30] = "127.0.0.1";

#pragma pack(1)
struct Server_Recv_Struct {
	BOOL KeyBuffer[256];
};
#pragma pack()

#pragma pack(1)
struct Server_Send_Struct {
	POINT pos[MAX_PLAYER];
};
#pragma pack()

Server_Recv_Struct SRS;
Server_Send_Struct SSS;

struct SOCKETINFO
{
	WSAOVERLAPPED overlapped;
	WSABUF dataBuffer;
	SOCKET socket;
	int receiveBytes;
	int sendBytes;
	BOOL KeyBuffer[256];
	POINT pos;
};

map <SOCKET, SOCKETINFO> clients;

void CALLBACK recv_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags);
void CALLBACK send_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags);

void err_quit(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	//MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCWSTR)msg, MB_ICONERROR);
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
//DWORD WINAPI ProcessClient(LPVOID arg);



int main()
{
	int retval;

	// Winsock Start - windock.dll 로드
	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0)
	{
		cout << "Error - Can not load 'winsock.dll' file\n";
		return 1;
	}

	// 1. 소켓생성  
	SOCKET listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	//SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket == INVALID_SOCKET) err_quit((char*)"socket()");

	// 서버정보 객체설정
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = PF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	// 2. 소켓설정
	if (bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		printf("Error - Fail bind\n");
		// 6. 소켓종료
		closesocket(listenSocket);
		// Winsock End
		WSACleanup();
		return 1;
	}

	// 3. 수신대기열생성
	if (listen(listenSocket, 5) == SOCKET_ERROR)
	{
		printf("Error - Fail listen\n");
		err_quit((char*)"listen()");
		// 6. 소켓종료
		closesocket(listenSocket);
		// Winsock End
		WSACleanup();
		return 1;
	}

	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(SOCKADDR_IN);
	memset(&clientAddr, 0, addrLen);
	SOCKET clientSocket;
	DWORD flags;

	HANDLE hThread;

	while (1)
	{
		cout << "억셉 대기중\n";
		// accept()
		clientSocket = accept(listenSocket, (struct sockaddr *)&clientAddr, &addrLen);
		if (clientSocket == INVALID_SOCKET)
		{
			err_display((char*)"accept()");
			break;
		}

		clients[clientSocket] = SOCKETINFO{};
		memset(&clients[clientSocket], 0x00, sizeof(struct SOCKETINFO));
		clients[clientSocket].socket = clientSocket;
		clients[clientSocket].dataBuffer.len = sizeof(clients[clientSocket].KeyBuffer);
		clients[clientSocket].dataBuffer.buf = (char*)&clients[clientSocket].KeyBuffer;
		flags = 0;


		clients[clientSocket].overlapped.hEvent = (HANDLE)clients[clientSocket].socket;

		if (WSARecv(clients[clientSocket].socket, &clients[clientSocket].dataBuffer, 1, NULL, &flags, &(clients[clientSocket].overlapped), recv_callback))
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				err_display("WSARecv");
				exit(1);
			}
		}
		else {
			cout << "Non Overlapped Recv return.\n";
			return 1;
		}

	}

	// 6-2. 리슨 소켓종료
	closesocket(listenSocket);

	// Winsock End
	WSACleanup();

	return 0;
}

void CALLBACK recv_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags)
{
	SOCKET client_s = reinterpret_cast<int>(overlapped->hEvent);

	DWORD sendBytes = 0;
	DWORD receiveBytes = 0;
	DWORD flags = 0;

	if (dataBytes == 0)
	{
		closesocket(clients[client_s].socket);
		clients.erase(client_s);
		return;
	}  // 클라이언트가 closesocket을 했을 경우


	//memcpy(&P1KeyBuffer, &clients[client_s].dataBuffer, sizeof(P1KeyBuffer));

	if (clients[client_s].KeyBuffer[VK_LEFT])
	{
		if (clients[client_s].pos.x > 0)
			clients[client_s].pos.x -= 100;
	}
	else if (clients[client_s].KeyBuffer[VK_RIGHT])
	{
		if (clients[client_s].pos.x < BOARDSIZE - 100)
			clients[client_s].pos.x += 100;
	}
	else if (clients[client_s].KeyBuffer[VK_DOWN])
	{
		if (clients[client_s].pos.y < BOARDSIZE - 100)
			clients[client_s].pos.y += 100;
	}
	else if (clients[client_s].KeyBuffer[VK_UP])
	{
		if (clients[client_s].pos.y > 0)
			clients[client_s].pos.y -= 100;
	}

	clients[client_s].dataBuffer.len = sizeof(clients[client_s].pos);
	clients[client_s].dataBuffer.buf = (char*)&clients[client_s].pos;

	memset(&(clients[client_s].overlapped), 0x00, sizeof(WSAOVERLAPPED));
	clients[client_s].overlapped.hEvent = (HANDLE)client_s;

	
	//memcpy(&clients[client_s].dataBuffer, &characterPos1, sizeof(characterPos1));

	
	if (WSASend(client_s, &(clients[client_s].dataBuffer), 1, &dataBytes, 0, &(clients[client_s].overlapped), send_callback) == SOCKET_ERROR)

	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			err_display("WSASend()");
			exit(1);
		}
	}

	clients[client_s].dataBuffer.len = MAX_BUFFER;
	clients[client_s].dataBuffer.buf = (char*)&clients[client_s].KeyBuffer;


	memset(&(clients[client_s].overlapped), 0x00, sizeof(WSAOVERLAPPED));
	clients[client_s].overlapped.hEvent = (HANDLE)client_s;
	if (WSARecv(client_s, &clients[client_s].dataBuffer, 1, &receiveBytes, &flags, &(clients[client_s].overlapped), recv_callback) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			err_display("WSARecv");
			exit(1);
		}
	}



}

void CALLBACK send_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags)
{
	DWORD sendBytes = 0;
	DWORD receiveBytes = 0;
	DWORD flags = 0;

	SOCKET client_s = reinterpret_cast<int>(overlapped->hEvent);

	if (dataBytes == 0)
	{
		closesocket(clients[client_s].socket);
		clients.erase(client_s);
		return;
	}  // 클라이언트가 closesocket을 했을 경우

	{
		// WSASend(응답에 대한)의 콜백일 경우
		
	}
}