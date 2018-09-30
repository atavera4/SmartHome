// SmartHomeProject.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

int main()
{

	MainServer server;
	while (true)
	{
		// use another thread to do these things:
		server.ProbeNetworkForActiveNodes();
		Sleep(100);
		//break;
		server.CheckNodeDataStreams();
	}

	while (true)
	{
		;
	}
	return 0;

	// to do list:
	/*
	1- finish the networking utils class and use it to replace all of the redundant code
	that does any tcp networking operations
	2 - Set Up the tcp link between this C++ process and the Go web server process
	3 - Finish function that checks to see of a remote node has been disconnected
	and if so tries to reconnect
	4 - implement motion detection class from python to C++ and add it to the
	motion detect class here
	5 - stuff on other list ...
	*/
}


