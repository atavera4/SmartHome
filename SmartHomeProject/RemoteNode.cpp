#include "stdafx.h"

// Wait for server to contact this node:
void RemoteIOT::ListenForServerProbe()
{
	// complete this handshake mechanism for the node and main server:
	// then on main server set up a data buffer class
	// the purpose of this buffer class is to accept data into a queue object and push it out 
	// to the Go web server frontend
	// if the queue reaches a certain size then clear it and continue accepting new data packets:
	// each node class will have its own data buffer class object: 
	// the main server class will periodically call each of the remote nodes objects 1 at a time
	// it will check the mutex of the nodes buffer object to see if queue is available:
	// if not it will move on to the next node in map of active node connections
	// if queue is availalbe then it will dump its buffer queue into the Go tcp link connection:

	// each thread will have an internal count of max items in buffer in order to 
	// prevent any possible starvation for thread
	// if buffer limit is reached the thread will stop and unlock its mutex and wait a certain time until main
	// server reads from it
	// if too much time has passed while in this state and main server 
	// still has not read the buffer then thread will empty its buffer anyway
	// and recieve new images and repeat the whole process:


	// initialize winsock:

	WSADATA ws_data;
	WORD ver = MAKEWORD(2, 2);
	int ws_ok = WSAStartup(ver, &ws_data);
	if (ws_ok != 0) {
		std::cerr << "Can't Initialize winsock! Quitting" << std::endl;
		return;
	}


	// create a socket:
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0); // use DGRAM FOR UDP
	if (listening == INVALID_SOCKET) {
		std::cerr << "INVALID SOCKET !!!" << std::endl;
		return;
	}

	// Bind the socket to an ip address and port:
	sockaddr_in hint;
	hint.sin_family = AF_INET; // NETWORKING IS BIG ENDIAN WHILE PC'S ARE LITTLE ENDIAN
	hint.sin_port = htons(54001);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;  // could also use inet_pton ....
	int err_code = bind(listening, (sockaddr*)&hint, sizeof(hint));
	if (listen(listening, SOMAXCONN) == SOCKET_ERROR)
	{
		std::cerr << "Error: cant listen on this socket" << std::endl;
		wprintf(L"listen failed with error: %ld\n", WSAGetLastError());
		closesocket(listening);
		WSACleanup();
		return;

	}


	// Tell winsock the socket is for listening:
	if (listen(listening, SOMAXCONN) == SOCKET_ERROR)
	{
		std::cerr << "Error: cant listen on this socket" << std::endl;
		wprintf(L"listen failed with error: %ld\n", WSAGetLastError());
		closesocket(listening);
		WSACleanup();
		return;

	}


	// Wait for a connection:
	sockaddr_in client;
	int client_size = sizeof(client);
	SOCKET client_socket = accept(listening, (sockaddr*)&client, &client_size);
	// can check if invalid socket here if need be ...
	if (client_socket == INVALID_SOCKET)
	{
		std::cout << "error" << std::endl;
	}
	int err = WSAGetLastError();


	char host[NI_MAXHOST]; // client's remote name
	char service[NI_MAXHOST]; // Service (i.e port) the client is connect on

	ZeroMemory(host, NI_MAXHOST); // Same as memset(host, 0, NI_MAXHOST);
	ZeroMemory(host, NI_MAXHOST); // ZeroMemory is just a define in WINDOWS

								  // try to look up the host name of the client.
								  // otherwise just get the ip address:  
	if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
	{
		std::cout << host << " connected on port " << service << std::endl;
	}
	else
	{
		inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
		std::cout << host << "connected on port " << ntohs(client.sin_port) << std::endl;
	}

	// close listening socket:
	closesocket(listening);

	this->initial_link_socket = client_socket;


	return;
}


bool RemoteIOT::DecodeServerMsg(char buff[])
{
	std::string buffer(buff);
	std::cout << "Msg recieved: " << buffer << std::endl;
	int len = buffer.size();
	std::string name = "";
	std::string ip = "";
	std::string port = "";

	int dash_seen = 0;
	for (int i = 0; i < len; i++)
	{
		std::cout << buffer[i] << std::endl;
		if (buffer[i] == '-')
		{
			dash_seen += 1;
			continue;
		}

		if (dash_seen == 0)
			name += buffer[i];
		else if (dash_seen == 1)
			ip += buffer[i];
		else if (dash_seen == 2)
			port += buffer[i];
	}

	if (name == "MainServer")
	{
		this->server_name = name;
		this->server_ip = ip;
		this->server_port = stoi(port);
		return true;
	}
	return false;
}

std::string RemoteIOT::EncodeServerLinkMsg()
{
	// encode using a custom version of karp rabin:
	// then add resulting number to each of the chars in the node_hash_key:
	// then do some xor sorcery: 
	return "";
}


void RemoteIOT::RecvMainServerConfirmation()
{
	char buffer[4096];
	int key;
	char buff[30];
	char *ptr = new char[921600];
	ZeroMemory(ptr, 921600);
	while (true)
	{
		ZeroMemory(buffer, 4096);

		// get confirmation message from main server containing
		// encrypted ip address, port and name strings:
		// the encyrption is a special hash function:
		// for now it is just every char incremented by +2:

		std::cout << "Waiting for message from main server" << std::endl;
		int info_bytes = recv(this->initial_link_socket, buff, 30, 0);
		if (info_bytes == SOCKET_ERROR)
		{
			std::cerr << "Error in recv(). Quitting" << std::endl;
			break;
		}
		if (info_bytes == 0)
		{
			std::cout << "Client disconnected" << std::endl;
			break;
		}
		/*
		The client connects to the server, sending in the user - name(but not password)
		The server responds by sending out unique random number
		The client encrypts that random number using the hash of their password as the key
		The client sends the encrypted random number to the server
		The server encrypts the random number with the correct hash of the user's password
		The server compares the two encrypted random numbers
		*/

		if (!this->DecodeServerMsg(buff))
		{
			// if not the right message then do not start a connection
			// wait 30 seconds then open port again for another possible connection attempt:
			Sleep(1000);
			continue;
		}
		else
		{
			this->ConnectToServer();
			this->StartVideoStream();
		}
	}

	return;
}








void RemoteIOT::ConnectToServer()
{
	std::string ip_addr = this->server_ip;
	int port = this->server_port;

	// Initialize winsock:
	WSADATA data;
	WORD ver = MAKEWORD(2, 2);
	int wsResult = WSAStartup(ver, &data);
	if (wsResult != 0)
	{
		std::cerr << "Can't start Winsock, Err # " << wsResult << std::endl;
		return;

	}

	// Create socket:
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		std::cerr << "Can't create socket, Err #" << WSAGetLastError() << std::endl;
		return;
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
			return;
		}
		else
		{
			Sleep(1000);
			timeout += 1;
			std::cout << "Waiting for server to atart up: seconds elapes: " << timeout << std::endl;
		}
	}
	std::cout << "Connected to Server !!!" << std::endl;


	this->connection_to_main_server = sock;

	return;
}


void RemoteIOT::StartVideoStream()
{
	cv::Mat img, imgGray;
	img = cv::Mat::zeros(480, 1920, CV_8U);
	//make it continuous
	if (!img.isContinuous()) {
		img = img.clone();
	}

	int imgSize = img.total() * img.elemSize();
	int bytes = 0;
	int key;

	std::cout << "Image Size:" << imgSize << std::endl;

	int capDev = 0;

	cv::VideoCapture cap(capDev); // open the default camera
	std::vector<uchar> jpg_data;

	while (1) {

		cap >> img;
		if (!img.isContinuous()) {
			img = img.clone();
		}
		imgSize = img.total() * img.elemSize();
		//usleep(50);
		cv::namedWindow("IOT", 0);
		//cv::namedWindow("IOT22", 1);
		//cv::imshow("IOT", img);
		if (key = cv::waitKey(1) >= 0) break;
		imgSize = img.total() * img.elemSize();





		std::string img_size_str = std::to_string(imgSize);

		// Send data here
		int bytes_sent = send(this->connection_to_main_server, img_size_str.c_str(), 6, 0);
		bytes_sent = 0;

		const int y = 9;


		char* img_bytes = (char*)img.data;

		std::ofstream myfile("IOT_DATA.txt");
		if (myfile.is_open())
		{
			myfile << "This is THE start.\n";
			myfile << img_bytes;
			myfile << "This is the END.\n";
			myfile.close();
		}
		else std::cout << "Unable to open file";

		std::cout << "Image Size:" << imgSize << std::endl;
		for (int i = 0; i < imgSize; i += bytes_sent)
		{
			bytes_sent = send(this->connection_to_main_server, img_bytes, imgSize, 0);
		}

		/*// Try having the server send the data back to this and populate the image data with it to display:
		// it may shed some light on what the data structure differences are besides just step ...
		// ONLY THING MISSING NOW IS THE VIDEO COLOR !!!
		// THEN MAYBE AUDIO IF NEEDED !!!

		// store data in buffer and recreate:
		// this is what happens on the other side:

		uchar *ptr = new uchar[imgSize];
		ZeroMemory(ptr, imgSize);
		memcpy(ptr, img_bytes, imgSize);

		cv::Mat new_img(480, 1920, CV_8U, (uchar*)ptr);
		//cv::Mat new_img = cv::Mat::zeros(480, 1920, CV_8U);
		if (!new_img.isContinuous())
		{
		new_img = new_img.clone();
		}
		// copy everything else:

		cout << "here" << endl;

		//memcpy(new_img.data, ptr, imgSize);
		int new_img_size = new_img.total() * new_img.elemSize();

		cout << "" << endl;

		//cv::imshow("IOT", new_img);*/
		cv::imshow("IOT", img);


	}
	return;
}

