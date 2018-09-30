#pragma once

// Utility abstract class that should be inherited by any class that
// needs to do any networking operations
// provides basic networking io functions which can be overrided 
// by child classes if more specific functionality is needed:
class NetworkingUtilities {
public:
	virtual std::pair<bool, SOCKET> CreateDataSenderConnection(std::string ip_addr, int port, std::string msg = "");
	virtual std::pair<bool, SOCKET> CreateDataRecieverConnection(std::string ip_addr, int port, std::string msg = "");
	virtual std::pair<bool, SOCKET> RecieveData(int buffer_size);

};


