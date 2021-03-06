// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
//#define _WIN32_WINNT 0x0501

#include "targetver.h"

#include <stdio.h>
//#include <iostream>
#include <tchar.h>
#include <string>
#include <opencv2/opencv.hpp>
#include <vector>
#include <map>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
//#include <WinSock2.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>


#pragma comment(lib, "WS2_32.lib")


// TODO: reference additional headers your program requires here
#include "NetworkingUtilities.h"
#include "RemoteDeviceConnectionManager.h" 
#include "MainServer.h"
#include "MotionDetect.h"
#include "RemoteNode.h"

