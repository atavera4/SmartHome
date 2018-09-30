#pragma once

// All remote device manager classes must have these functions:
class RemoteNode {
public:
	virtual bool StartDataStreamConnection() = 0;
	virtual void CloseConnection() = 0;
	virtual void SetNodeId(int id) = 0;
	virtual void PushToDataBuffer(char* data) = 0;
};