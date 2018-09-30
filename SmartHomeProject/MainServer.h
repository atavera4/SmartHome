#pragma once



// Main server class is repsonsible for listening for any new tcp connection requests:
// if a valid request is recieved a new sub class is created which manages the new thread
// for the specific device connection:
class MainServer {
public:
	MainServer()
	{
		device_id = 0;
		this->server_ip = "127.0.0.1";
		this->search_port = 54000;
		// load registered nodes/devices from the DB:
		std::string name = "local dev";
		std::string ip = "127.0.0.1";
		int port = 54001;
		NodeInfo dev_node(name, ip, port);
		std::pair<std::string, NodeInfo> node_key_pair(name, dev_node);
		this->reg_nodes_map.insert(node_key_pair);
	};
	~MainServer()
	{
		// delete all remote devce manager pointer entries in the map:
		for (auto it = devices_map.begin(); it != devices_map.end(); ++it)
		{
			RemoteDeviceConnectionManager* dev_mangr = it->second;
			delete dev_mangr;
		}
	}

	// searches local network for remote devices ready to send video:
	// Main server acts as a client trying to connect to the nodes acting as 'servers':
	// Using the registered ip_addresses of these nodes:
	void ProbeNetworkForActiveNodes();
	bool SearchForSpecificNode(NodeInfo &node_info);
	bool InitVideoFeed(NodeInfo node_info);
	void CheckNodeDataStreams();
	void CloseNodeConnection();

	// need to add function to check whether the remote nodes have stopped transmitting:
	void CheckNodeStatus();
	void StartWebServerConnection();
	void ConnectToDataBase();
	std::pair<bool, SOCKET> CreateDataSenderConnection(std::string ip_addr, int port, std::string msg = "");

private:
	// map of device connection managers:
	// each takes care of starting its own thread:
	// the main server should take care of managing these threads
	// to prevent any conflicts:
	std::string server_ip;
	int search_port;
	int device_id;
	std::map<std::string, NodeInfo> reg_nodes_map;
	std::map<std::string, int> devices_id_map;
	std::map<std::string, RemoteDeviceConnectionManager*> devices_map;
	std::mutex mtx;
	// info for Golang web server this process will connect to:
	int Go_web_server_port;
	std::string Go_web_server_ip;
	SOCKET Go_web_server_socket;
};



