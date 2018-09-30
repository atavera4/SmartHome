#pragma once


// Simple contianer for collection of variables/info pulled from Database:
struct NodeInfo {
	std::string node_name;
	std::string node_ip_addr;
	int node_port;
	bool is_active;
	NodeInfo(std::string name, std::string ip_address, int port)
	{
		this->is_active = false;
		this->node_name = name;
		this->node_ip_addr = ip_address;
		this->node_port = port;
	}
};

// Utility class that should be inherited by any class that
// needs to do any networking operations:
class NetworkingUtilities {
public:
	virtual std::pair<bool, SOCKET> CreateDataSenderConnection(std::string ip_addr, int port, std::string msg = "");
	virtual std::pair<bool, SOCKET> CreateDataRecieverConnection(std::string ip_addr, int port, std::string msg = "");
	virtual std::pair<bool, SOCKET> RecieveData(int buffer_size);

};

// All remote device manager classes must have these functions:
class RemoteNode {
public:
	virtual bool StartDataStreamConnection() = 0;
	virtual void CloseConnection() = 0;
	virtual void SetNodeId(int id) = 0;
	virtual void PushToDataBuffer(char* data) = 0;
};


// this class is used for handling video streaming nodes.
// will change name to something better later on:
class RemoteDeviceConnectionManager : RemoteNode {
public:

	RemoteDeviceConnectionManager(NodeInfo node_info, int node_id)
	{
		// store this nodes unique id:
		this->SetNodeId(node_id);
		this->node_name = node_info.node_name;
		this->node_ip_address = node_info.node_ip_addr;
		this->node_port = node_info.node_port;
		this->stop_thread_flag = false;
		this->data_buffer_q = std::queue<uchar*>();
		this->main_thread_is_here = false;
		this->data_buffer_size = 0;
	};

	~RemoteDeviceConnectionManager()
	{

		// should make this bool atomin and implement a condition variable:
		this->stop_thread_flag = true;
		// join the thread so that this blocks until thread sees the stop flag
		// and exits before the object that the thread is a member of is deleted
		// this will prevent errors:
		this->thread_obj->join();
		this->CloseConnection();
	}

	void Test() { ; }
	virtual bool StartDataStreamConnection();
	virtual void PushToDataBuffer(char* data);
	virtual void CloseConnection();
	void SetNodeId(int id);
	bool DisplayImage(char* image);
	bool StartServer();
	void RecvVideo();
	int node_id;
	// data buffer queue used to store incomming data
	// this data is then read and removed by main thread:
	std::queue<uchar*> data_buffer_q;
	std::atomic<bool> stop_thread_flag;
	std::atomic<int> data_buffer_size;
	// mutex used to handle access to member threads' data buffer by the main thread;
	std::mutex mtx;
	// condition variable is used when main thread needs to access buffer but needs to
	// make the thread wait:
	std::condition_variable cond_var;
	// used for when main thread needs access to child thread:
	// this will inform the child thread that it needs to wait on cond_var:
	std::atomic<bool> main_thread_is_here;

private:
	std::string node_name;
	std::string node_ip_address;
	int node_port;
	std::thread* thread_obj;
	// atomic flag used to make sure that when this object is destroyed
	// its thread member is exited:
	SOCKET device_connection_socket;
};




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



