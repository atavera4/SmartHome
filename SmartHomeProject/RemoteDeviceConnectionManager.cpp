#include "stdafx.h"

void RemoteDeviceConnectionManager::CloseConnection()
{
	closesocket(this->device_connection_socket);
	WSACleanup();
	return;
}

void RemoteDeviceConnectionManager::SetNodeId(int id)
{
	this->node_id = id;
}

bool RemoteDeviceConnectionManager::StartDataStreamConnection()
{
	// Establish a connection:
	if (!this->StartServer())
	{
		std::cerr << "RemoteDeviceConnectionManager::StartStreamConnection() UNSUCCSESSFULL !!!" << std::endl;
		return false;
	}
	// begin new thread for recieving data through socket:
	this->thread_obj = new std::thread(&RemoteDeviceConnectionManager::RecvVideo, this);
	//thread_obj->join();
	return true;
}

void RemoteDeviceConnectionManager::RecvVideo()
{
	char buffer[4096];
	char buff[6];
	char *ptr = new char[921600];
	ZeroMemory(ptr, 921600);
	while (true)
	{
		if (this->stop_thread_flag)
		{
			delete ptr;
			this->stop_thread_flag = false;
			return;
		}

		ZeroMemory(buffer, 4096);
		ZeroMemory(ptr, 921600);
		// get size of incomming image frame
		int info_bytes = recv(this->device_connection_socket, buff, 6, 0);
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
		int image_size = atoi(buff);
		int bytes_recieved;
		for (int i = 0; i < image_size; i += bytes_recieved)
		{
			bytes_recieved = recv(this->device_connection_socket, ptr, image_size, MSG_WAITALL);
			if (bytes_recieved == SOCKET_ERROR)
			{
				std::cerr << "Error in recv(). Quitting" << std::endl;
				break;
			}
			if (bytes_recieved == 0)
			{
				std::cout << "Client disconnected" << std::endl;
				break;
			}
		}
		this->PushToDataBuffer(ptr);
		if (this->DisplayImage(ptr))
			break;
	}
	delete ptr;
	return;
}

bool RemoteDeviceConnectionManager::StartServer()
{
	// initialize winsock:
	WSADATA ws_data;
	WORD ver = MAKEWORD(2, 2);
	int ws_ok = WSAStartup(ver, &ws_data);
	if (ws_ok != 0) {
		std::cerr << "Can't Initialize winsock! Quitting" << std::endl;
		return false;
	}

	// create a socket:
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0); // use DGRAM FOR UDP
	if (listening == INVALID_SOCKET) {
		std::cerr << "INVALID SOCKET !!!" << std::endl;
		return false;
	}
	// Bind the socket to an ip address and port:
	sockaddr_in hint;
	hint.sin_family = AF_INET; // NETWORKING IS BIG ENDIAN WHILE PC'S ARE LITTLE ENDIAN
	hint.sin_port = htons(this->node_port);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;  // could also use inet_pton ....
	bind(listening, (sockaddr*)&hint, sizeof(hint));
	// Tell winsock the socket is for listening:
	listen(listening, SOMAXCONN);
	// Wait for a connection:
	sockaddr_in client;
	int client_size = sizeof(client);
	SOCKET client_socket = accept(listening, (sockaddr*)&client, &client_size);
	// can check if invalid socket here if need be ...
	if (client_socket == INVALID_SOCKET)
	{
		std::cout << "error" << std::endl;
	}
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
	this->device_connection_socket = client_socket;
	return true;
}


void RemoteDeviceConnectionManager::PushToDataBuffer(char* data)
{
	//uchar* c = new uchar(921600);
	uchar c[921600];
	memcpy(c, data, 921600);
	// if buffer has x pieces of data in it then empty buffer and continue:
	if (this->data_buffer_size > 100)
	{
		//delete stuff;
		;
	}

	// acquire the mutex:
	std::unique_lock<std::mutex> lk(this->mtx);
	// using wait_for here to handle
	// 3 possible cases: 

	// Deadlock situation where main thread got mutex first and sent notify before this got to its wait and the mutex is not 
	// locked/unlocked by main thread at all after this wait unlocks it so its waiting forever
	// wait_for prevents this child thread from waiting forever.

	// wait is called first and we dont want to wait more then 'x' time for main thread to finish and unlock/notify
	// in which case wait_for will call lock and make this thread wait until the main thread unlocks the mutex anyway:

	// or main thread is taking a long time doing something else and doesnt take the mutext for a while
	// so instead of having this thread wait for main thread it will just timeout and take control of the mutex again
	// if the main thread takes control of mutext before wait_for can lock it again after timeout
	//then the child thread will
	// just wait until main thread unlockes mutex and proper behavior is still assured.
	this->cond_var.wait_for(lk, std::chrono::milliseconds(1));
	this->data_buffer_q.push(c);
	this->data_buffer_size += 1;


	return;
}

bool RemoteDeviceConnectionManager::DisplayImage(char* image_data)
{
	int key;
	uchar ptr2[921600];
	memcpy(ptr2, image_data, 921600);
	//cv::Mat new_img = cv::Mat::zeros(480, 1920, CV_8U);
	cv::Mat new_img(480, 1920, CV_8U, (uchar*)ptr2);
	if (!new_img.isContinuous())
	{
		new_img = new_img.clone();
	}
	int new_img_size = new_img.total() * new_img.elemSize();
	cv::Mat n = new_img.clone();
	cvtColor(new_img, n, CV_GRAY2RGB);
	cv::namedWindow("Server", 0);
	cv::imshow("Server", n);
	//delete ptr;
	if (key = cv::waitKey(1) >= 0)
		return true;

	return false;
}