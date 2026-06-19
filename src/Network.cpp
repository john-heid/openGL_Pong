#include "Network.h";
#include <WinSock2.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

static SOCKET sock = INVALID_SOCKET;
static sockaddr_in remote_addr{};
static bool is_server = false;
bool NetworkInitServer(int port)
{
	WSADATA wsaData{};
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cout << "WSAStartup failed" << std::endl;
		return false;
	}

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == INVALID_SOCKET) {
		std::cout << "Socket creation failed" << std::endl;
		return false;
	}

	sockaddr_in server_addr{};
	server_addr.sin_family = AF_INET;

}