// Peer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

DWORD WINAPI UserCLI(LPVOID);

SOCKET connectedPeer[1024];
char connectedName[1024][256];
SOCKADDR_IN knownPeer[1024];
char peerName[1024][256];
int connectedAmount = 0;
int knownAmount = 0;

int main()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	int tmpport = 9001;

	SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	while (true)
	{
		addr.sin_port = htons(tmpport);
		int ret = bind(listener, (SOCKADDR FAR*)&addr, sizeof(addr));
		if (ret == 0)
		{
			break;
		}
		if (tmpport == 65000)
		{
			return -1;
		}
		tmpport++;
	}
	listen(listener, 1024);

	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(9000);
	SOCKET connector = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	char user[256];
	printf("Nhap username: ");
	fgets(user, 255, stdin);

	int c = connect(connector, (SOCKADDR FAR*)&addr, sizeof(addr));

	if (c < 0)
	{
		printf("Error: %d\n", GetLastError());
	}

	char buf[4096];

	printf("Peer: %hu\n", tmpport);

	sprintf(buf, "%hu %s\n", htons(tmpport), user);
	send(connector, buf, strlen(buf), 0);

	fd_set readfds;

	CreateThread(0, 0, UserCLI, NULL, 0, 0);

	timeval timelimit = { 0, 1 };

	while (true)
	{
		FD_ZERO(&readfds);
		FD_SET(listener, &readfds);
		FD_SET(connector, &readfds);
		for (int i = 0; i < connectedAmount; i++)
		{
			FD_SET(connectedPeer[i], &readfds);
		}

		int ret = select(0, &readfds, NULL, NULL, NULL);

		if (ret < 0)
		{
			printf("Error: %d\n", GetLastError());
			continue;
		}

		if (ret == 0)
		{
			continue;
			printf("Timeout!\n");
		}

		for (int i = 0; i < connectedAmount; i++)
		{
			if (FD_ISSET(connectedPeer[i], &readfds))
			{
				int len = recv(connectedPeer[i], buf, sizeof(buf), 0);
				if (len <= 0)
				{
					connectedAmount--;
					printf("Peer %d disconnected!\n", connectedPeer[i]);
					closesocket(connectedPeer[i]);
					connectedPeer[i] = connectedPeer[connectedAmount];
					strcpy(connectedName[i], connectedName[connectedAmount]);
				}
				buf[len] = 0;
				printf("%s", buf);
			}
		}

		if (FD_ISSET(listener, &readfds))
		{
			SOCKADDR_IN addr = { 0 };
			int len = sizeof(addr);
			connectedPeer[connectedAmount] = accept(listener, (SOCKADDR FAR*)&addr, &len);
			
			printf("Peer %d connected!\n", connectedPeer[connectedAmount]);
			connectedAmount++;
		}

		if (FD_ISSET(connector, &readfds))
		{
			int len = recv(connector, buf, sizeof(buf), 0);
			char tmpaddr[32];
			char tmpname[256];
			unsigned short tmpport;
			sscanf(buf, "%s%hu%s", tmpaddr, &tmpport, tmpname);

			knownPeer[knownAmount].sin_addr.s_addr = inet_addr(tmpaddr);
			knownPeer[knownAmount].sin_port = tmpport;
			knownPeer[knownAmount].sin_family = AF_INET;
			strcpy(peerName[knownAmount], tmpaddr);
			knownAmount++;
		}
	}

	WSACleanup();
	return 0;
}

DWORD WINAPI UserCLI(LPVOID lpParam)
{
	char buf[4096];
	while (true)
	{
		fgets(buf, 4096, stdin);
		for (int i = 0; i < knownAmount; i++)
		{
			SOCKET connector = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			connect(connector, (SOCKADDR FAR*)&knownPeer[i], sizeof(knownPeer[i]));
			send(connector, buf, strlen(buf), 0);
			closesocket(connector);
		}
	}
}