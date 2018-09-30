#pragma once

class RemoteIOT {
public:
	RemoteIOT() {};
	void ListenForServerProbe();
	void RecvMainServerConfirmation();
	std::string EncodeServerLinkMsg();
	bool DecodeServerMsg(char buffer[]);
	void ConnectToServer();
	void StartVideoStream();
private:
	std::string server_name;
	std::string server_ip;
	int server_port;
	std::string node_hash_key;
	SOCKET initial_link_socket;
	SOCKET connection_to_main_server;
};

