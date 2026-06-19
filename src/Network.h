#pragma once

struct GameState {
	float ball_x, ball_y;
	float p1_y, p2_y;
	int p1_score, p2_score;
};

struct PlayerInput {
	bool up, down;
};

bool NetworkInitServer(int port);
bool NetworkInitClient(const char* server_ip, int port);
void NetworkSend(const void* data, int size);
bool NetworkReceive(void* buffer, int size);
void NetworkShutdown();
