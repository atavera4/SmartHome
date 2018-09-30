
#include "stdafx.h"

bool MainServer::InitVideoFeed(NodeInfo node_info)
{
	std::string name = node_info.node_name;

	RemoteDeviceConnectionManager* rem_dev = new RemoteDeviceConnectionManager(node_info, this->device_id);

	if (!rem_dev->StartDataStreamConnection())
	{
		return false;
	}

	std::pair<std::string, RemoteDeviceConnectionManager*> d(name, rem_dev);
	std::pair<std::string, int> d_id(name, this->device_id);
	this->devices_id_map.insert(d_id);
	this->devices_map.insert(d);
	this->device_id += 1;
	return true;
}


void MainServer::ProbeNetworkForActiveNodes()
{

	std::cout << "Probing netowrk for active nodes ..." << std::endl;
	// for each node in list of registered remote nodes:
	for (auto it = this->reg_nodes_map.begin(); it != this->reg_nodes_map.end(); ++it)
	{
		//NodeInfo node_info = it->second;
		//this->InitVideoFeed(node_info);
		// no need to look for a remote device already connected:
		if (!it->second.is_active)
		{
			bool found = this->SearchForSpecificNode(it->second);
			if (found)
			{
				if (this->InitVideoFeed(it->second))
				{
					it->second.is_active = true;
				}
			}
		}
	}
	return;
}


bool MainServer::SearchForSpecificNode(NodeInfo &node_info)
{
	// Try to connect to this registered device listening on standard port #:
	std::string name = node_info.node_name;
	std::string ip_addr = node_info.node_ip_addr;// "127.0.0.1";
	int port = node_info.node_port;
	std::cout << "Attempting to locate device: " << name << " with ip_addr: "
		<< ip_addr << " on port: " << port << std::endl;
	std::string msg = "MainServer-" + this->server_ip + "-" + std::to_string(port) + "-";
	std::pair<bool, SOCKET> s = this->CreateDataSenderConnection(ip_addr, port, msg);
	return s.first;
}


// This function is called to periodically check
//  all of the data stream buffers for each of the
// remote device manager objects with active connections
// at a regular interval:
void MainServer::CheckNodeDataStreams()
{
	std::map<std::string, RemoteDeviceConnectionManager*>* active_devices = &this->devices_map;
	for (auto it = active_devices->begin(); it != active_devices->end(); it++)
	{
		// check if the objects thread mutex is locked for its queue buffer:
		RemoteDeviceConnectionManager* rem_dev = it->second;
		std::cout << "Main Thread:  reading and clearing remote node datastream buffer/queue ..." << std::endl;
		// read and empty the buffer:
		if (rem_dev->data_buffer_q.size() > 10)
		{
			std::unique_lock<std::mutex> lk(rem_dev->mtx);
			// empty buffer
			int buff_size = rem_dev->data_buffer_size;
			for (int i = 0; i < buff_size; i++)
			{
				//delete rem_dev->data_buffer_q.front();
				rem_dev->data_buffer_q.pop();
			}
			rem_dev->data_buffer_size = 0;
			rem_dev->cond_var.notify_one();
		}
	}
	return;
}


void MainServer::CloseNodeConnection()
{
	std::map<std::string, RemoteDeviceConnectionManager*>* active_devices = &this->devices_map;
	for (auto it = active_devices->begin(); it != active_devices->end(); it++)
	{
		// check if the objects thread mutex is locked for its queue buffer:
		std::cout << "Main Thread is closing device connection" << std::endl;
		// kill the stream:
		RemoteDeviceConnectionManager* rem_dev = it->second;
		Sleep(5000);
		delete rem_dev;
		// delete entry from map as well:
		active_devices->erase(it->first);
	}

	return;
}


void MainServer::CheckNodeStatus()
{
	return;
}



void MainServer::StartWebServerConnection()
{

	return;
}

void MainServer::ConnectToDataBase()
{

	return;
}



std::pair<bool, SOCKET> MainServer::CreateDataSenderConnection(std::string ip_addr, int port, std::string msg)
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