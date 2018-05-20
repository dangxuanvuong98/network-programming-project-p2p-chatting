// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include<iostream>
#include <tchar.h>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "winsock2.h"
#include <time.h>

int InitializeListener();
DWORD WINAPI UserCLI(LPVOID);
DWORD WINAPI HandleDownload(LPVOID lpParam);
DWORD WINAPI HandleRequest(LPVOID lpParam);


extern SOCKET listener;
extern unsigned short listenerPort;
extern SOCKET connector;
extern SOCKET connectedPeer[1024];
extern SOCKADDR_IN knownPeer[1024];
extern int connectedAmount;
extern int knownAmount;
extern bool connectedToTracker;
extern char fileList[128][256];
extern int fileLength[128];
extern int fileAmount;
extern bool local[128][1024];
extern timeval timelimit;