// Peer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

int main()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (InitializeListener() < 0)
	{
		return -1;
	}

	CreateThread(0, 0, UserCLI, NULL, 0, 0);

	char buf[4096];

	fd_set readfds;
	connectedAmount = 0;

	while (true)
	{
		FD_ZERO(&readfds);
		FD_SET(listener, &readfds);
		if (connectedToTracker)
		{
			FD_SET(connector, &readfds);
		}

		for (int i = 0; i < connectedAmount; i++)
		{
			FD_SET(connectedPeer[i], &readfds);
		}

		int ret = select(0, &readfds, NULL, NULL, &timelimit);

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
					//printf("Peer %d disconnected!\n", connectedPeer[i]);
					closesocket(connectedPeer[i]);
					connectedPeer[i] = connectedPeer[connectedAmount];
				}
				buf[len] = 0;
				char param[4096];
				sprintf(param, "%d %s", connectedPeer[i], buf);
				HandleRequest(param);
				while (param[0] != 0);
			}
		}

		if (FD_ISSET(listener, &readfds))
		{
			connectedPeer[connectedAmount] = accept(listener, NULL, NULL);
			//printf("Peer %d connected!\n", connectedPeer[connectedAmount]);
			connectedAmount++;
		}

		if (FD_ISSET(connector, &readfds))
		{
			int len = recv(connector, buf, sizeof(buf), 0);

			if (len < 0)
			{
				closesocket(connector);
				connectedToTracker = false;
			}

			char tmpaddr[32];
			unsigned short tmpport;
			sscanf(buf, "%s%hu", tmpaddr, &tmpport);

			knownPeer[knownAmount].sin_addr.s_addr = inet_addr(tmpaddr);
			knownPeer[knownAmount].sin_port = tmpport;
			knownPeer[knownAmount].sin_family = AF_INET;
			knownAmount++;
		}
	}

	WSACleanup();
	return 0;
}
