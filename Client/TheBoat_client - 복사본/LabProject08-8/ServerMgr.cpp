#include "stdafx.h"
#include "GameFramework.h"
#include "ServerMgr.h"

void ServerMgr::ErrorDisplay(const char* msg, int err_no) {
	_wsetlocale(LC_ALL, L"korean");
	WCHAR *lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("%s", msg);
	wprintf(L"%s\n", lpMsgBuf);
	LocalFree(lpMsgBuf);
}

void ServerMgr::IPInput() {
	while (true) {
		cout << "서버 아이피 입력 : ";
		cin >> server_ip;
		break;
	}
}

void ServerMgr::Initialize(HWND& hwnd) {
	WSADATA	wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
	int opt_val = TRUE;
	setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*)&opt_val, sizeof(opt_val));

	SOCKADDR_IN ServerAddr;
	ZeroMemory(&ServerAddr, sizeof(SOCKADDR_IN));
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(SERVER_PORT);
	// 아이피
	ServerAddr.sin_addr.s_addr = inet_addr(server_ip.c_str());
	
	//ServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	
	int retval = WSAConnect(sock, (sockaddr *)&ServerAddr, sizeof(ServerAddr), NULL, NULL, NULL, NULL);
	if (retval == SOCKET_ERROR) {
		printf("소켓 연결 안됨\n");
	}
	async_handle = hwnd;
	WSAAsyncSelect(sock, async_handle, WM_SOCKET, FD_CONNECT | FD_CLOSE | FD_READ);

	send_wsabuf.buf = send_buffer;
	send_wsabuf.len = CLIENT_BUF_SIZE;
	recv_wsabuf.buf = recv_buffer;
	recv_wsabuf.len = CLIENT_BUF_SIZE;
	printf("server_mgr 초기화\n");
}

void ServerMgr::ReadPacket() {
	DWORD io_bytes, io_flag = 0;

	int retval = WSARecv(sock, &recv_wsabuf, 1, &io_bytes, &io_flag, NULL, NULL);
	if (retval == 1) {
		int err_code = WSAGetLastError();
		ErrorDisplay("[WSARecv] : 에러 ", err_code);
	}
	BYTE* ptr = reinterpret_cast<BYTE*>(recv_buffer);

	while (io_bytes != 0) {
		if (in_packet_size == 0)
			in_packet_size = ptr[0];
		if (io_bytes + saved_packet_size >= in_packet_size) {
			memcpy(packet_buffer + saved_packet_size, ptr, in_packet_size - saved_packet_size);
			ProcessPacket(packet_buffer);
			ptr += in_packet_size - saved_packet_size;
			io_bytes -= in_packet_size - saved_packet_size;
			in_packet_size = 0;
			saved_packet_size = 0;
		}
		else {
			memcpy(packet_buffer + saved_packet_size, ptr, io_bytes);
			saved_packet_size += io_bytes;
			io_bytes = 0;
		}
	}
}
Bullet ServerMgr::GetBullet() {
	return bullets[recvd_bullet_id];
}

Box ServerMgr::GetBox(int index) {
	return boxes[index];
}

int ServerMgr::GetClientID() {
	return clients_id;
}

void ServerMgr::ProcessPacket(char* ptr) {
	static bool first_time = true;
	switch (ptr[1]) {
	case SC_ENTER_PLAYER: {

		SC_PACKET_ENTER_PLAYER * packets = reinterpret_cast<SC_PACKET_ENTER_PLAYER*>(ptr);
		if (first_set_id) {
			clients_id = packets->id;
			camera_id = packets->id;
			first_set_id = false;
		}
		sc_vec_buff[packets->id].pos.x = packets->x;
		sc_vec_buff[packets->id].pos.y = packets->y;
		sc_vec_buff[packets->id].pos.z = packets->z;

		sc_vec_buff[packets->id].elecX = packets->elecX;
		sc_vec_buff[packets->id].elecY = packets->elecY;
		sc_vec_buff[packets->id].elecZ = packets->elecZ;

		if(packets->id == clients_id)
			elecPos = XMFLOAT3(packets->elecX, packets->elecY, packets->elecZ);
		break;
	}

	case SC_PLAYER_LOGIN: {
		cout << "서버로긴패킷" << endl;
		SC_PACKET_LOGIN_PLAYER* packets = reinterpret_cast<SC_PACKET_LOGIN_PLAYER*>(ptr);
		clients_id = packets->id;
		strncpy_s((char*)sc_vec_buff[packets->id].playerID, maxUserIDLen, packets->userid, maxUserIDLen);
		strncpy_s((char*)loginID[packets->id], maxUserIDLen, packets->userid, maxUserIDLen);
		
		for (int i = 0; i < 4; ++i)
			cout << loginID[i] << endl;
		cout << sc_vec_buff[packets->id].playerID << "로그인했당" << endl;

		break;
	}

	case SC_PLAYER_CHAT: {
		SC_PACKET_CHAT* packets = reinterpret_cast<SC_PACKET_CHAT*>(ptr);
		clients_id = packets->id;
		strncpy_s((char*)sc_vec_buff[packets->id].chat, maxChatSize, packets->chat, maxChatSize);

		cout << sc_vec_buff[packets->id].chat << "채팅 " << endl;
		strncpy_s(chatchar,maxChatSize, sc_vec_buff[packets->id].chat, maxChatSize);
		chatPlayerIndex = packets->id;
		newChat = true;
		break;

	}


	case SC_BUILDING_GEN: {
		SC_PACKET_ENTER_PLAYER* packets = reinterpret_cast<SC_PACKET_ENTER_PLAYER*>(ptr);
		building_pos[packets->id].x = packets->x;
		building_pos[packets->id].y = packets->y;
		building_pos[packets->id].z = packets->z;

		building_extents[packets->id].x = packets->size_x;
		building_extents[packets->id].y = packets->size_y;
		building_extents[packets->id].z = packets->size_z;
		//printf("[%d] 빌딩 [%f, %f, %f] 크기 : [%f, %f, %f] \n", packets->id,
		//	building_pos[packets->id].x,
		//	building_pos[packets->id].y,
		//	building_pos[packets->id].z,
		//	building_extents[packets->id].x,
		//	building_extents[packets->id].y,
		//	building_extents[packets->id].z);
		break;
	}

	case SC_POS: {
		SC_PACKET_POS* packets = reinterpret_cast<SC_PACKET_POS*>(ptr);
		clients_id = packets->id;
		sc_vec_buff[packets->id].pos.x = packets->x;
		sc_vec_buff[packets->id].pos.y = packets->y;
		sc_vec_buff[packets->id].pos.z = packets->z;
		//sc_vec_buff[packets->id].is_die = packets->is_die;
		sc_look_vec = packets->look_vec;

		// 0 숨쉬기, 1: 걷기, 2: 뛰기
		sc_vec_buff[packets->id].player_status = packets->player_status;
		if (packets->is_die) {
			sc_vec_buff[packets->id].player_status = 17;
		}
		//sc_vec_buff[packets->id].elecCount = packets->elecCount;
		//elecCount = packets->elecCount;

		//printf("elecCount : %d\n", packets->elecCount);
		

		break;
	}
	case SC_IS_DIE: {
		SC_PACKET_IS_DIE* packets = reinterpret_cast<SC_PACKET_IS_DIE*>(ptr);
		clients_id = packets->id;
		sc_look_vec = packets->look_vec;
		sc_vec_buff[packets->id].is_die = packets->is_die;

		// 0 숨쉬기, 1: 걷기, 2: 뛰기
		sc_vec_buff[packets->id].player_status = packets->player_status;
		if (packets->is_die) {
			sc_vec_buff[packets->id].player_status = 17;
		}
		//sc_vec_buff[packets->id].elecCount = packets->elecCount;
		//elecCount = packets->elecCount;

		//printf("elecCount : %d\n", packets->elecCount);


		break;
	}
	case SC_PLAYER_LOOKVEC: {
		SC_PACKET_LOOCVEC* packets = reinterpret_cast<SC_PACKET_LOOCVEC*>(ptr);
		clients_id = packets->id;
		sc_look_vec = packets->look_vec;
		sc_vec_buff[packets->id].player_status = packets->player_status;
		elecCount = packets->elecCount;

		break;
	}
	case SC_BULLET_POS: {
		SC_PACKET_BULLET* packets = reinterpret_cast<SC_PACKET_BULLET*>(ptr);
		clients_id = packets->id;
		recvd_bullet_id = packets->bullet_id;
		bullets[packets->bullet_id].id = packets->bullet_id;
		bullets[packets->bullet_id].x = packets->x;
		bullets[packets->bullet_id].y = packets->y;
		bullets[packets->bullet_id].z = packets->z;

		//printf("[Bullet] %d 플레이어 총알 ID[%d] \n", clients_id, packets->bullet_id);
		break;
	}

	case SC_BOX_POS: {
		SC_PACKET_BOX* packets = reinterpret_cast<SC_PACKET_BOX*>(ptr);
		clients_id = packets->id;
		recvd_box_id = packets->box_id;
		//boxes[clients_id][recvd_box_id].id = packets->box_id;
		//boxes[clients_id][recvd_box_id].x = packets->x;
		//boxes[clients_id][recvd_box_id].y = packets->y;
		//boxes[clients_id][recvd_box_id].z = packets->z;
		boxes[recvd_box_id].id = clients_id * MAX_BOX_SIZE;    // 클라 * 10(십의자리 인덱스) + 박스
		boxes[recvd_box_id].x = packets->x;
		boxes[recvd_box_id].y = packets->y;
		boxes[recvd_box_id].z = packets->z;
		boxes[recvd_box_id].hp = packets->hp;
		boxes[recvd_box_id].in_use = packets->in_use;
		myBoxCount = packets->boxCount[camera_id];
		printf("난 %d번 클라 남은 박스는 %d개\n", camera_id, myBoxCount);


		//printf("[Bullet] %d 플레이어 총알 ID[%d] \n", clients_id, packets->bullet_id);
		break;
	}
	case SC_COLLSION_PB: {
		SC_PACKET_COLLISION* packets = reinterpret_cast<SC_PACKET_COLLISION*>(ptr);
		collision_pos.x = packets->x;
		collision_pos.y = packets->y;
		collision_pos.z = packets->z;
		s_is_collide = true;
		damageCheck = true;
		client_hp[packets->client_id] = packets->hp;
		sndPlaySound(L"../Assets/Sounds/TakeDamage.wav", SND_ASYNC);

		printf("%d 플레이어의 충돌지점 x : %f, y : %f, z : %f, 체력 : %f \n", packets->client_id, collision_pos.x,
			collision_pos.y, collision_pos.z, client_hp[packets->client_id]);
		break;
	}
	case SC_COLLSION_BB: {
		SC_PACKET_COLLISION_BB* packets = reinterpret_cast<SC_PACKET_COLLISION_BB*>(ptr);
		collision_box_pos.x = packets->x;
		collision_box_pos.y = packets->y;
		collision_box_pos.z = packets->z;
		boxes[packets->box_id].in_use = packets->in_use;
		if (!packets->in_use) {
			sndPlaySound(L"../Assets/Sounds/BrokenBox.wav", SND_ASYNC);
			particlepos = collision_box_pos;
			particlepos.y -= 8;
			treeparticle = true;
		}
		printf("부딪 박스 %d\n", packets->box_id);
		box_is_collide = true;
		box_hp[packets->box_id] = packets->hp;

		/*if(box_hp[packets->box_id] < 0){
			boxes[packets->box_id].x = 0;
			boxes[packets->box_id].z = 0;
		}*/
		/*printf("%d 플레이어의 충돌지점 x : %f, y : %f, z : %f, 체력 : %f \n", packets->client_id, collision_box_pos.x,
			collision_box_pos.y, collision_box_pos.z, box_hp[packets->client_id]);*/

		break;
	}
	case SC_COLLSION_OB: {
		SC_PACKET_COLLISION_OB* packets = reinterpret_cast<SC_PACKET_COLLISION_OB*>(ptr);
		collision_obj_pos.x = packets->x;
		collision_obj_pos.y = packets->y;
		collision_obj_pos.z = packets->z;
		obj[packets->obj_id].in_use = packets->in_use;
		if (packets->obj_id % 3 == 0 && !packets->in_use) {
			obj[packets->obj_id].item_tree = true;
		}
		if (!packets->in_use) {
			sndPlaySound(L"../Assets/Sounds/BrokenTree.wav", SND_ASYNC);
			particlepos = collision_obj_pos;
			treeparticle = true;
		}
		printf("부딪 나무 %d\n", packets->obj_id);
		obj_is_collide = true;
		obj_hp[packets->obj_id] = packets->hp;
		
		break;
	}
	case SC_COLLSION_BDP: {	// building to player
		SC_PACKET_COLLISION* packets = reinterpret_cast<SC_PACKET_COLLISION*>(ptr);
		collision_pos.x = packets->x;
		collision_pos.y = packets->y;
		collision_pos.z = packets->z;
		//client_hp[packets->client_id] = packets->hp;
		printf("%d 플레이어 벽과 꽈당 [%f, %f, %f] \n", packets->client_id, collision_pos.x,
			collision_pos.y, collision_pos.z, client_hp[packets->client_id]);

		break;
	}
	case SC_ITEM_GEN: {
		// 아이템 생성
		SC_PACKET_ITEM_GEN* packets = reinterpret_cast<SC_PACKET_ITEM_GEN*>(ptr);
		item_pos.x = packets->x;
		item_pos.y = packets->y;
		item_pos.z = packets->z;
		printf("아템 생성\n");
		is_item_gen = true;
		break;
	}
	case SC_READY: {
		SC_PACKET_READY* packets = reinterpret_cast<SC_PACKET_READY*>(ptr);
		for (int k = 0; k < MAX_PLAYER_SIZE; ++k)
		{
			player_ready[k] = packets->player_ready[k];
			if (player_ready[k])
				printf("%d 레디 완료\n", k);
		}
		game_start = packets->game_start;
		if (game_start)
			printf("게임시작 ㄱㄱㄱ\n");
		break;
	}

	case SC_BOX_COUNT: {
		SC_PACKET_BOX_COUNT* packets = reinterpret_cast<SC_PACKET_BOX_COUNT*>(ptr);
		myBoxCount = packets->boxcount;
		break;
	}
	}
}
int ServerMgr::GetElecCount() {
	return elecCount;
}
float ServerMgr::GetPlayerHP(int p_n) {
	return client_hp[p_n];

}
bool ServerMgr::IsItemGen() {
	return is_item_gen;
}

void ServerMgr::ReturnBuildingPosition(XMFLOAT3* input_building_pos) {
	for (int i = 0; i < OBJECT_BUILDING; ++i) {
		input_building_pos[i].x = building_pos[i].x;
		input_building_pos[i].y = building_pos[i].y;
		input_building_pos[i].z = building_pos[i].z;
	}
}

void ServerMgr::ReturnBuildingExtents(XMFLOAT3* input_building_extents) {
	for (int i = 0; i < OBJECT_BUILDING; ++i) {
		input_building_extents[i].x = building_extents[i].x;
		input_building_extents[i].y = building_extents[i].y;
		input_building_extents[i].z = building_extents[i].z;
	}
}


XMFLOAT3 ServerMgr::ReturnItemPosition() {
	is_item_gen = false;
	return item_pos;
}

XMFLOAT3 ServerMgr::ReturnCollsionPosition(bool* is_collide) {
	*is_collide = s_is_collide;
	s_is_collide = false;
	return collision_pos;
}

void ServerMgr::SendDeadPacket() {
	CS_PACKET_KEYUP* packet_buffer = reinterpret_cast<CS_PACKET_KEYUP*>(send_buffer);
	packet_buffer->size = sizeof(CS_PACKET_KEYUP);
	send_wsabuf.len = sizeof(CS_PACKET_KEYUP);
	int retval = 0;
	DWORD iobytes;
	packet_buffer->type = CS_PLAYER_DIE;
	packet_buffer->isPlayerdead[camera_id] = isplayerdead[camera_id];
	retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);

	if (retval == 1) {
		int error_code = WSAGetLastError();
		ErrorDisplay("[WSASend] 에러 : ", error_code);
	}
}

void ServerMgr::SendRootPacket(int type) {
	CS_PACKET_ROOT_ITEM* packet_buffer = reinterpret_cast<CS_PACKET_ROOT_ITEM*>(send_buffer);
	packet_buffer->size = sizeof(CS_PACKET_ROOT_ITEM);
	send_wsabuf.len = sizeof(CS_PACKET_ROOT_ITEM);
	int retval = 0;
	DWORD iobytes;
	
	if (type == TYPE_BOX)
		packet_buffer->type = CS_ROOT_BOX;
	/*else if (type == TYPE_BULLET)
		packet_buffer->type = CS_ROOT_BULLET;*/
	else 
		packet_buffer->type = CS_ROOT_ITEM;

	packet_buffer->skill = type;
	retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
	if (retval == 1) {
		int error_code = WSAGetLastError();
		ErrorDisplay("[WSASend] 에러 : ", error_code);
	}
}

void ServerMgr::SendPacket(int type) {
	CS_PACKET_KEYUP* packet_buffer = reinterpret_cast<CS_PACKET_KEYUP*>(send_buffer);
	packet_buffer->size = sizeof(CS_PACKET_KEYUP);
	send_wsabuf.len = sizeof(CS_PACKET_KEYUP);
	int retval = 0;
	DWORD iobytes;

	switch (type) {
	case CS_KEY_PRESS_UP:
		packet_buffer->type = CS_KEY_PRESS_UP;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_KEY_PRESS_DOWN:
		packet_buffer->type = CS_KEY_PRESS_DOWN;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_KEY_PRESS_RIGHT:
		packet_buffer->type = CS_KEY_PRESS_RIGHT;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_KEY_PRESS_LEFT:
		packet_buffer->type = CS_KEY_PRESS_LEFT;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_KEY_PRESS_CROUCH:
		packet_buffer->type = CS_KEY_PRESS_CROUCH;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_KEY_PRESS_SHIFT:
		packet_buffer->type = CS_KEY_PRESS_SHIFT;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_KEY_PRESS_SPACE:
		packet_buffer->type = CS_KEY_PRESS_SPACE;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_KEY_PRESS_1:
		packet_buffer->type = CS_KEY_PRESS_1;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_KEY_PRESS_2:
		packet_buffer->type = CS_KEY_PRESS_2;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_KEY_PRESS_Q:
		packet_buffer->type = CS_KEY_PRESS_Q;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;


	case CS_KEY_RELEASE_UP:
		packet_buffer->type = CS_KEY_RELEASE_UP;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_KEY_RELEASE_DOWN:
		packet_buffer->type = CS_KEY_RELEASE_DOWN;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_KEY_RELEASE_RIGHT:
		packet_buffer->type = CS_KEY_RELEASE_RIGHT;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_KEY_RELEASE_LEFT:
		packet_buffer->type = CS_KEY_RELEASE_LEFT;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_KEY_RELEASE_CROUCH:
		packet_buffer->type = CS_KEY_RELEASE_CROUCH;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;

	case CS_KEY_RELEASE_SHIFT:
		packet_buffer->type = CS_KEY_RELEASE_SHIFT;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_KEY_RELEASE_SPACE:
		packet_buffer->type = CS_KEY_RELEASE_SPACE;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_KEY_RELEASE_1:
		packet_buffer->type = CS_KEY_RELEASE_1;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_KEY_RELEASE_2:
		packet_buffer->type = CS_KEY_RELEASE_2;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_KEY_RELEASE_Q:
		packet_buffer->type = CS_KEY_RELEASE_Q;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;



	case CS_LEFT_BUTTON_DOWN:
		packet_buffer->type = CS_LEFT_BUTTON_DOWN;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_RIGHT_BUTTON_DOWN:
		packet_buffer->type = CS_RIGHT_BUTTON_DOWN;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;

	case CS_LEFT_BUTTON_UP:
		packet_buffer->type = CS_LEFT_BUTTON_UP;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_RIGHT_BUTTON_UP:
		packet_buffer->type = CS_RIGHT_BUTTON_UP;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;

	case CS_MOUSE_MOVE:
		packet_buffer->type = CS_MOUSE_MOVE;
		// 여기에 추가적으로 player의 look 벡터를 같이 해서 보내줘야한다. 
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
	case CS_PLAYER_READY:
		packet_buffer->type = CS_PLAYER_READY;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_PLAYER_READY_CANCLE:
		packet_buffer->type = CS_PLAYER_READY_CANCLE;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	}

	if (retval == 1) {
		int error_code = WSAGetLastError();
		ErrorDisplay("[WSASend] 에러 : ", error_code);
	}

}


void ServerMgr::SendPacket(int type, char* id) {
	CS_PACKET_LOBBY* packet_buffer = reinterpret_cast<CS_PACKET_LOBBY*>(send_buffer);
	packet_buffer->size = sizeof(CS_PACKET_LOBBY);
	send_wsabuf.len = sizeof(CS_PACKET_LOBBY);
	int retval = 0;
	DWORD iobytes;
	switch (type) {
	case CS_PLAYER_LOGIN:
		packet_buffer->type = CS_PLAYER_LOGIN;
		strncpy_s((char *)packet_buffer->userID, maxUserIDLen, id, maxUserIDLen);

		cout << "패킷보내 ID" << packet_buffer->userID << endl;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;

	case CS_PLAYER_CHAT:
		packet_buffer->type = CS_PLAYER_CHAT;
		strncpy_s((char *)packet_buffer->chatbuffer, maxChatSize, id, maxChatSize);

		cout << "패킷보내 채팅 " << packet_buffer->chatbuffer << endl;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);

		break;


	}

	if (retval == 1) {
		int error_code = WSAGetLastError();
		ErrorDisplay("[WSASend] 에러 : ", error_code);
	}

}

void ServerMgr::SendCameraPacket(float x, float y, float z, XMFLOAT3 look_vec) {
	CS_PACKET_CAMERA* packet_buffer = reinterpret_cast<CS_PACKET_CAMERA*>(send_buffer);
	packet_buffer->size = sizeof(CS_PACKET_CAMERA);
	send_wsabuf.len = sizeof(CS_PACKET_CAMERA);
	int retval = 0;
	DWORD iobytes;
	packet_buffer->type = CS_CAMERA;
	packet_buffer->cameraX = x;
	packet_buffer->cameraY = y;
	packet_buffer->cameraZ = z;
	packet_buffer->camera_look_vec = look_vec;

	retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);

	if (retval == 1) {
		int error_code = WSAGetLastError();
		ErrorDisplay("[WSASend] 에러 : ", error_code);
	}
}


void ServerMgr::SendPacket(int type, XMFLOAT3& xmvector) {
	CS_PACKET_KEYUP* packet_buffer = reinterpret_cast<CS_PACKET_KEYUP*>(send_buffer);
	packet_buffer->size = sizeof(CS_PACKET_KEYUP);
	send_wsabuf.len = sizeof(CS_PACKET_KEYUP);
	int retval = 0;
	DWORD iobytes;
	switch (type) {
	case CS_KEY_PRESS_UP:
		packet_buffer->type = CS_KEY_PRESS_UP;
		packet_buffer->look_vec = xmvector;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_KEY_PRESS_DOWN:
		packet_buffer->type = CS_KEY_PRESS_DOWN;
		packet_buffer->look_vec = xmvector;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_KEY_PRESS_RIGHT:
		packet_buffer->type = CS_KEY_PRESS_RIGHT;
		packet_buffer->look_vec = xmvector;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_KEY_PRESS_LEFT:
		packet_buffer->type = CS_KEY_PRESS_LEFT;
		packet_buffer->look_vec = xmvector;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_KEY_PRESS_CROUCH:
		packet_buffer->type = CS_KEY_PRESS_CROUCH;
		packet_buffer->look_vec = xmvector;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_KEY_PRESS_SHIFT:
		packet_buffer->type = CS_KEY_PRESS_SHIFT;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_KEY_PRESS_SPACE:
		packet_buffer->type = CS_KEY_PRESS_SPACE;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_KEY_PRESS_1:
		packet_buffer->type = CS_KEY_PRESS_1;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_KEY_PRESS_2:
		packet_buffer->type = CS_KEY_PRESS_2;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_KEY_PRESS_Q:
		packet_buffer->type = CS_KEY_PRESS_Q;
		packet_buffer->look_vec = xmvector;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_KEY_RELEASE_UP:
		packet_buffer->type = CS_KEY_RELEASE_UP;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_KEY_RELEASE_DOWN:
		packet_buffer->type = CS_KEY_RELEASE_DOWN;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_KEY_RELEASE_RIGHT:
		packet_buffer->type = CS_KEY_RELEASE_RIGHT;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_KEY_RELEASE_LEFT:
		packet_buffer->type = CS_KEY_RELEASE_LEFT;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_KEY_RELEASE_CROUCH:
		packet_buffer->type = CS_KEY_RELEASE_CROUCH;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_KEY_RELEASE_SHIFT:
		packet_buffer->type = CS_KEY_RELEASE_SHIFT;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_KEY_RELEASE_SPACE:
		packet_buffer->type = CS_KEY_RELEASE_SPACE;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_KEY_RELEASE_1:
		packet_buffer->type = CS_KEY_RELEASE_1;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_KEY_RELEASE_2:
		packet_buffer->type = CS_KEY_RELEASE_2;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_KEY_RELEASE_Q:
		packet_buffer->type = CS_KEY_RELEASE_Q;
		packet_buffer->look_vec = xmvector;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;


	case CS_LEFT_BUTTON_DOWN:
		packet_buffer->type = CS_LEFT_BUTTON_DOWN;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_RIGHT_BUTTON_DOWN:
		packet_buffer->type = CS_RIGHT_BUTTON_DOWN;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;

	case CS_LEFT_BUTTON_UP:
		packet_buffer->type = CS_LEFT_BUTTON_UP;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;
	case CS_RIGHT_BUTTON_UP:
		packet_buffer->type = CS_RIGHT_BUTTON_UP;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
		break;

	case CS_MOUSE_MOVE:
		packet_buffer->type = CS_MOUSE_MOVE;
		// 여기에 추가적으로 player의 look 벡터를 같이 해서 보내줘야한다.
		packet_buffer->look_vec = xmvector;
		retval = WSASend(sock, &send_wsabuf, 1, &iobytes, 0, NULL, NULL);
	}
	if (retval == 1) {
		int error_code = WSAGetLastError();
		ErrorDisplay("[WSASend] 에러 : ", error_code);
	}

}


void ServerMgr::ClientError() {
	exit(-1);
}

SPlayer ServerMgr::ReturnPlayerPosStatus(int client_id) {
	return sc_vec_buff[client_id];
}

XMFLOAT3 ServerMgr::ReturnLookVector() {
	return sc_look_vec;
}
int ServerMgr::ReturnCameraID() {
	return camera_id;
}