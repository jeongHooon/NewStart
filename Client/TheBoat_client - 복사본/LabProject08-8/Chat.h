#pragma once

#include <algorithm>

#include <iostream>
using namespace std;

#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")

#include <string.h>

// 서버에 접속할 ip와 port
const char serverIP[] = "127.0.0.1";
const int serverPort = 12345;


//const int maxUserIDLen = 20;
//const int maxPasswdLen = 20;
//const int maxChatSize = 256;
// 먼저 모든 프로토콜에서 공통되는 부분
struct ProtoCommand
{
	WORD command;
	BYTE data[0];
};


const WORD ComLoginREQ = 1; // id와 패스워드로 로긴 요청
struct StrLoginREQ    //프로토콜에 REQ가 붙으면
{						//  뭔가 요청을 하는 패킷
	BYTE userid[maxUserIDLen];
};

const WORD ComLoginACK = 2; // 서버에서 로긴결과 응답
struct StrLoginACK	// 프로토콜에 ACK가 붙으면
{					// REQ에 대한 응답을 의미하는 패킷
	BYTE result;
	// 0이면 로긴 성공, 아니면 실패
};
enum EnumLoginACK
{
	LoginACKConnectAllow = 0,		// 접속 허용
	LoginACKInvalidPasswd = 1,		// 패스워드 불일치
	LoginACKDuplicateConnect = 2,	// 한 아이디로 중복접속
};
// 나중에 다른 프로토콜의 에러메시지가 많아지거나
// 하면 다른 프로토콜의 에러와 헷갈리는 경우가
// 많이 생기므로 이렇게 같은 enum의 값들은
// 같은 형식으로 만드는 것이 좋다.

//-----------------------여기까지

// 채팅용프로토콜
const WORD ComChatREQ = 3;//서버로 문자열 보낸다. REQ
struct StrChatREQ
{
	BYTE chat[maxChatSize];
};

const WORD ComChatACK = 4;//서버에서는 성공 여부 반환 ACK
struct StrChatACK
{
	BYTE result; // 0이면 성공, 아니면 실패
};
enum EnumChatACK
{
	ChatACKSuccess = 0,		// 채팅성공
	ChatACKNotLogin = 1,	// 아직 로긴중
};

const WORD ComChatCMD = 5;//성공시 모든 클라이언트로 문자열 보낸다. CMD
struct StrChatCMD
{
	BYTE userid[maxUserIDLen];
	BYTE chat[maxChatSize];
};

//귓속말용 프로토콜을 설계한다.
const WORD ComWisperREQ = 6;// 누군가에게 귓속말 보내달라고 요청
struct StrWisperREQ
{
	BYTE toWhom[maxUserIDLen];  // 받을 사람 id
	//BYTE chat[maxChatSize];
	BYTE chat[0];
};
// 귓말로 채팅을 단 한글자(1바이트)만 보내더라도 패킷 전체의
//크기(maxChatSize)를 다 보내버리기 때문에 패킷의 낭비가 생길 수
//있다. 그러므로 필요한 만큼만 보낸다면 패킷의 낭비를 막을 수 있다.
// chat의 크기를 0으로 잡는다면 chat 자체는 위치만 가지고 있을 뿐
//크기는 가지지 않게 된다.
// 그러므로 패킷을 보낼 때 이 구조체 크기에 chat의 실제 길이를
//더해서 보낸다면 꼭 필요한 만큼의 패킷을 보내 통신량을 줄일 수
//있다.(참고 : 클라이언트의 SendWisper() 메서드)

const WORD ComWisperACK = 7;
struct StrWisperACK
{
	BYTE result;
};
enum EnumWisperACK
{
	WisperACKSuccess,
	WisperACKNotLogin, // 로긴되지 않은 상태에서 귓말을 보내려 할 경우
	WisperACKNotFound, // 상대를 찾을 수 없을 경우
};

const WORD ComWisperCMD = 8;
struct StrWisperCMD
{
	BYTE userID[maxUserIDLen];
	BYTE chat[maxChatSize];
};


/* 감정 표현 패킷 */
const WORD ComFeelingREQ = 10;
struct StrFeelingREQ
{
	BYTE userID[maxUserIDLen];
	BYTE chat[maxChatSize];
};

const WORD ComFeelinACK = 11;
struct StrFeelingACK
{
	BYTE result;
};

enum EnumFeelingACK
{
	FeelingACKSuccess = 0,
	FeelingACKNotLogin = 1,
};

const WORD ComFeelingCMD = 12;
struct StrFeelingCMD
{
	BYTE userid[maxUserIDLen];
	BYTE chat[maxChatSize];
};
