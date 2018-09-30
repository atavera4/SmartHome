#include "stdafx.h"



std::pair<bool, SOCKET> NetworkingUtilities::CreateDataSenderConnection(std::string ip_addr,
	int port, std::string msg)
{
	// Try to connect to host specified:
	std::cout << "Attempting to locate device:  with ip_addr: "
		<< ip_addr << " on port: " << port << std::endl;

	// Initialize winsock:
	WSADATA data;
	WORD ver = MAKEWORD(2, 2);
	int wsResult = WSAStartup(ver, &data);
	if (wsResult != 0)
	{
		std::cerr << "Can't start Winsock, Err # " << wsResult << std::endl;
		return std::pair<bool, SOCKET>(false, NULL);

	}

	// Create socket:
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		std::cerr << "Can't create socket, Err #" << WSAGetLastError() << std::endl;
		return std::pair<bool, SOCKET>(false, NULL);
	}

	// Fill in hint structure:
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	inet_pton(AF_INET, ip_addr.c_str(), &hint.sin_addr);

	// Connect to server:
	int connResult = connect(sock, (sockaddr*)&hint, sizeof(hint));
	int timeout = 0;
	while (connResult == SOCKET_ERROR)
	{
		connResult = connect(sock, (sockaddr*)&hint, sizeof(hint));
		//std::cerr << "Can't connect to server, Err #" << WSAGetLastError() << std::endl;
		if (timeout >= 10)
		{
			closesocket(sock);
			WSACleanup();
			return std::pair<bool, SOCKET>(false, NULL);
		}
		else
		{
			Sleep(1000);
			timeout += 1;
			std::cout << "Waiting for server to start up: seconds elapes: " << timeout << std::endl;
		}
	}
	std::cout << "Successfully found service and connected to it !!!" << std::endl;
	std::cout << "Sending stuff ..." << std::endl;

	// Send data here
	if (msg == "")
	{
		int bytes_sent = send(sock, "MainServer-", msg.size(), 0);
	}
	else
	{
		int bytes_sent = send(sock, msg.c_str(), msg.size(), 0);
	}
	closesocket(sock);
	WSACleanup();
	return std::pair<bool, SOCKET>(true, sock);
}


//
//std::pair<bool, SOCKET> NetworkingUtilities::CreateDataRecieverConnection(std::string ip_addr,
//	int port, std::string msg = "")
//{
//	return;
//}
//
//
//std::pair<bool, SOCKET> NetworkingUtilities::RecieveData(int buffer_size)
//{
//	return;
//}