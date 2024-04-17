#pragma once
#pragma comment(lib,"ws2_32.lib")   // Window소켓 라이브러리 링크
//#pragma comment(lib,"winmm.lib")    // Window 멀티미디어 관련 API 포함

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define BUFSIZE 1024
#define SERVERPORT 50002

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <thread>
#include <cstring>
#include <mysql.h>

#include "packet.h"
#include "main.h"

using namespace std;


struct DBConnection
{
	const char* server = "localhost",
		* user = "root",
		* pw = "1234",
		* db = "erin_db";
}db_conn;

MYSQL mysql;

// 소켓 관련 전역 변수
static SOCKET listen_sock;

// 서버를 죽일 때 사용하는 플래그
static bool dead_flag = false;

// 총 소켓 수를 체크하는 변수
int total_sockets = 0;

int scene = 1;
int check = 0;
char read_buf[1024];
// 소켓 함수 오류 출력 후 종류
void err_quit(const char* msg)
{
	LPVOID lp_msg_buf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lp_msg_buf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lp_msg_buf, msg, MB_ICONERROR);
	LocalFree(lp_msg_buf);
	exit(1);
}

// 소켓 함수 오류 출력
void err_display(const char* msg)
{
	LPVOID lp_msg_buf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lp_msg_buf, 0, NULL);
	printf("[%s] %s", msg, (char*)lp_msg_buf);
	LocalFree(lp_msg_buf);

}

void packet_proc(SOCKET sock, char* p_packet)
{
	PktRes res;
	res.pack_id = (short)PACKET_ID::RES;
	res.total_size = (short)sizeof(PktRes);
	res.num = 0;

	PktHeader* p_header = (PktHeader*)p_packet;

	if (p_header->pack_id == (short)PACKET_ID::REGISTER)
	{
		
	}
	else if (p_header->pack_id == (short)PACKET_ID::LOGIN)
	{

	}
}

void login_proc(SOCKET cli_sock ,char* buf)
{
	char* id = NULL;
	char* pw = NULL;
	id = strtok_s(buf, "/", &pw);

	cout << "Recv ID : " << id << endl;
	cout << "Recv PW : " << pw << endl;

	vaildate_log_id(cli_sock, id);
	if (check == 0)
		return;
	vaildate_log_pw(cli_sock, id, pw);
	if (check == 0)
		return;

	send(cli_sock, "0", 1, 0);
	scene = 0;
}

void vaildate_log_id(SOCKET cli_sock, char* id)
{
	char query[1024];
	int rowCount;
	MYSQL_RES* result;
	MYSQL_ROW row;

	sprintf_s(query, "SELECT COUNT(*) FROM erin_db.user_info WHERE id = '%s'", id);
	if (!mysql_query(&mysql, query))
	{
		cout << "아이디 존재여부 확인 성공" << endl;
	}
	else
	{
		cout << "존재 검사 실패\n에러 원인 :" << mysql_error(&mysql) << endl;
	}
	result = mysql_store_result(&mysql);
	row = mysql_fetch_row(result);
	rowCount = std::stoi(row[0]);
	if (rowCount > 0)
	{
		check = 1;
		return;
	}
	send(cli_sock, "1", 1, 0);
	check = 0;
}

void vaildate_log_pw(SOCKET cli_sock, char* id, char* pw)
{
	char query[1024];
	int rowCount;
	MYSQL_RES* result;
	MYSQL_ROW row;

	sprintf_s(query, "SELECT pw FROM erin_db.user_info WHERE id = '%s'", id);
	if (!mysql_query(&mysql, query))
	{
		cout << "비밀번호 찾기 성공" << endl;
	}
	else
	{
		cout << "비밀번호 찾기 실패\n에러 원인 :" << mysql_error(&mysql) << endl;
	}
	result = mysql_store_result(&mysql);
	row = mysql_fetch_row(result);
	rowCount = std::stoi(row[0]);

	if (rowCount == atoi(pw))
	{
		check = 1;
		return;
	}
	send(cli_sock, "2", 1, 0);
	check = 0;
}

void register_proc(SOCKET cli_sock, char* buf)
{
	char* nick = NULL;
	char* id = NULL;
	char* pw = NULL;
	char query[1024];
	
	nick = strtok_s(buf, "/", &id);
	cout << "nick : " << nick << endl;
	cout << "남은데이터 : " << id << endl;
	id = strtok_s(id, "/", &pw);
	cout << "id : " << id << endl;
	cout << "pw : " << pw << endl;


	vaildate_nickname(cli_sock,nick);
	if (check == 0)
		return;
	vaildate_id(cli_sock, id);
	if (check == 0)
		return;

	sprintf_s(query, "INSERT INTO `erin_db`.`user_info` (`name`,`id`,`pw`) VALUES ('%s','%s','%s')", nick, id, pw);
		
	if (!mysql_query(&mysql, query))
	{
		cout << "회원가입 성공" << endl;
		send(cli_sock, "0", 1, 0);
	}
	else
	{
		cout << "회원가입 실패\n에러 원인 :"<< mysql_error(&mysql) << endl;
	}
}

void vaildate_nickname(SOCKET cli_sock, char* nick)
{
	char query[1024];
	int rowCount;
	MYSQL_RES* result;
	MYSQL_ROW row;
	
	// 닉네임 중복 검사
	sprintf_s(query, "SELECT COUNT(*) FROM erin_db.user_info WHERE name = '%s'", nick);
	if (!mysql_query(&mysql, query))
	{
		cout << "닉네임 중복 검사 성공" << endl;
	}
	else
	{
		cout << "닉네임 중복 검사 실패\n에러 원인 :" << mysql_error(&mysql) << endl;
	}	
	result = mysql_store_result(&mysql);
	row = mysql_fetch_row(result);
	rowCount = std::stoi(row[0]);
	if (rowCount > 0)
	{
		send(cli_sock, "1", 1, 0);
		check = 0;
		return;
	}
	check = 1;
}

void vaildate_id(SOCKET cli_sock, char* id)
{
	char query[1024];
	int rowCount;
	MYSQL_RES* result;
	MYSQL_ROW row;

	// 닉네임 중복 검사
	sprintf_s(query, "SELECT COUNT(*) FROM erin_db.user_info WHERE id = '%s'", id);
	if (!mysql_query(&mysql, query))
	{
		cout << "아이디 중복 검사 성공" << endl;
	}
	else
	{
		cout << "닉네임 검사 실패\n에러 원인 :" << mysql_error(&mysql) << endl;
	}
	result = mysql_store_result(&mysql);
	row = mysql_fetch_row(result);
	rowCount = std::stoi(row[0]);
	if (rowCount > 0)
	{
		send(cli_sock, "2", 1, 0);
		check = 0;
		return;
	}
	check = 1;
}

void post_proc(SOCKET cli_sock, char* buf)
{
	char* title = NULL;
	char* author = NULL;
	char* content = NULL;
	char query[1024];

	title = strtok_s(buf, "/", &author);
	cout << "title : " << title << endl;
	cout << "남은데이터 : " << author << endl;
	author = strtok_s(author, "/", &content);
	cout << "author : " << author << endl;
	cout << "content : " << content << endl;

	sprintf_s(query, "INSERT INTO `erin_db`.`post` (`id`,`title`,`author`,`content`) VALUES ('%d','%s','%s','%s')", rand(), title, author, content);

	if (!mysql_query(&mysql, query))
	{
		cout << "게시글 성공" << endl;
		send(cli_sock, "0", 1, 0);
	}
	else
	{
		cout << "게시글 실패\n에러 원인 :" << mysql_error(&mysql) << endl;
	}
}

void open_proc(SOCKET cli_sock)
{
	char query[1024];
	MYSQL_RES* result;
	MYSQL_ROW row;

	sprintf_s(query, "SELECT COUNT(*) FROM erin_db.post");

	if (!mysql_query(&mysql, query))
	{
		cout << "게시글 개수 조회 성공" << endl;
		send(cli_sock, "0", 1, 0);
	}
	else
	{
		cout << "게시글 개수 조회\n에러 원인 :" << mysql_error(&mysql) << endl;
	}

	result = mysql_store_result(&mysql);
	row = mysql_fetch_row(result);
	/*send(cli_sock, row[0], 1, 0);*/
	int idx = atoi(row[0]);

	for (int i = 0; i < idx; ++i)
	{
		sprintf_s(query, "SELECT * FROM erin_db.post LIMIT 1 OFFSET %d", i);
		if (!mysql_query(&mysql, query))
		{
			send(cli_sock, "0", 1, 0);
		}
		else
		{
			cout << "게시글 만들기\n에러 원인 :" << mysql_error(&mysql) << endl;
		}
		result = mysql_store_result(&mysql);
		row = mysql_fetch_row(result);
		char buffer[1024];
		sprintf_s(buffer, "%s/%s/%s/%s/", row[0], row[1], row[2], row[3]);
		send(cli_sock, buffer, strlen(buffer), 0);
	}

}

void make_proc(SOCKET cli_sock, char* buf)
{
	char query[1024];
	char* id = NULL;
	char* gar = NULL;
	id = strtok_s(buf, "/", &gar);
	cout << "id :" << id << endl;
	cout << "id :" << gar << endl;
	MYSQL_RES* result;
	MYSQL_ROW row;

	sprintf_s(query, "SELECT * FROM erin_db.post WHERE id = '%s'", id);
	if (!mysql_query(&mysql, query))
	{
		cout << "게시글 만들기 성공" << endl;
		send(cli_sock, "0", 1, 0);
	}
	else
	{
		cout << "게시글 만들기\n에러 원인 :" << mysql_error(&mysql) << endl;
	}
	result = mysql_store_result(&mysql);
	row = mysql_fetch_row(result);
	char buffer[1024];
	sprintf_s(buffer, "%s/%s/%s/%s/", row[0], row[1], row[2], row[3]);
	send(cli_sock, buffer, strlen(buffer), 0);

}


void p_proc(SOCKET cli_sock, char* buf)
{
	char* p_id;
	char* data = NULL;
	p_id = strtok_s(buf, "/", &data);

	switch (stoi(p_id))
	{
	case static_cast<int>(PACKET_ID::REGISTER):
		register_proc(cli_sock,data);
		break;
	case static_cast<int>(PACKET_ID::LOGIN):
		login_proc(cli_sock,data);
		break;
	case static_cast<int>(PACKET_ID::POST):
		post_proc(cli_sock, data);
		break;
	case static_cast<int>(PACKET_ID::OPENPOST):
		open_proc(cli_sock);
		break;
	case static_cast<int>(PACKET_ID::MAKEPOST):
		make_proc(cli_sock, data);
		break;
	default:
		break;
	}

}



int main(int argc, char* argv[])
{
	srand((unsigned int)time(NULL));
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;
	// mysql 연결
	mysql_init(&mysql);
	if (mysql_real_connect(&mysql, db_conn.server, db_conn.user, db_conn.pw, db_conn.db, 50001, NULL, 0))
	{
		cout << "MySQL 버젼 : " << mysql_get_client_info() << endl;
		mysql_set_character_set(&mysql, "utf8");
	}
	else
		cout << "MySQL 연결 실패 \n에러 원인 : "<< mysql_error(&mysql) << endl;

	// sock()
	listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET)
		err_quit("Socket()");
	// bind()
	SOCKADDR_IN serv_addr;
	ZeroMemory(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(SERVERPORT);
	retval = ::bind(listen_sock, (SOCKADDR*)&serv_addr, sizeof(serv_addr));
	if (retval == SOCKET_ERROR)
		err_quit("Bind()");
	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR)
		err_quit("Listen()");
	while (1)
	{
		SOCKADDR_IN cli_addr;
		int addr_len = sizeof(cli_addr);

		SOCKET cli_sock = accept(listen_sock, (SOCKADDR*)&cli_addr, &addr_len);
		if (cli_sock == INVALID_SOCKET)
		{
			err_display("Accept()");
			break;
		}
		// 접속한 클라이언트 정보 출력
		char cli_ip[33] = { 0, };
		inet_ntop(AF_INET, &(cli_addr.sin_addr), cli_ip, 33 - 1);
		printf("\n[TCP 서버] 클라이언트 접속 : IP 주소 = %s, 포트번호 : %d\n", cli_ip, ntohs(cli_addr.sin_port));

		// 클라이언트와 데이터 통신
		char buf[BUFSIZE + 1];
		
		while (scene)
		{
			int recv_size = recv(cli_sock, buf, BUFSIZE, 0);
			string msg = buf;
			if (recv_size == SOCKET_ERROR)
			{
				err_display("Recv()");
				break;
			}
			if(recv_size > 0)
				buf[recv_size] = '\0';

			printf("[recv : %d]\n", recv_size);
			cout << buf << endl;
			p_proc(cli_sock, buf);
		}
		scene = 1;
		closesocket(cli_sock);
		printf("[TCP 서버] 클라이언트 종료 : IP 주소 : %s, 포트 번호 : %d\n", cli_ip, ntohs(cli_addr.sin_port));
	}

	closesocket(listen_sock);
	mysql_close(&mysql);
	WSACleanup();
	return 0;
}
