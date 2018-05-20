
#include "stdafx.h"

SOCKET listener;
unsigned short listenerPort = 9001;
SOCKET connector;
SOCKET connectedPeer[1024];
SOCKADDR_IN knownPeer[1024];
int connectedAmount = 0;
int knownAmount = 0;
bool connectedToTracker = false;
char fileList[128][256];
int fileLength[128];
int fileAmount = 0;
bool local[128][1024];
timeval timelimit = { 10, 0 };

DWORD WINAPI UserCLI(LPVOID lpParam)
{
	char buf[4096];
	char cmd[128];
	int offset;
	while (true)
	{
		fgets(buf, 4096, stdin);
		if (buf[0] == '\'')
		{
			strcpy(buf, buf + 1);
			for (int i = 0; i < knownAmount; i++)
			{
				SOCKET connector = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				connect(connector, (SOCKADDR FAR*)&knownPeer[i], sizeof(knownPeer[i]));
				send(connector, buf, strlen(buf), 0);
				closesocket(connector);
			}
		}
		else
		{
			sscanf(buf, "%s%n", cmd, &offset);
			if (strcmp(cmd, "chat") == 0)
			{
				char sendBuf[4096];
				strcpy(sendBuf, buf + offset + 1);

				for (int i = 0; i < knownAmount; i++)
				{
					SOCKET connector = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
					connect(connector, (SOCKADDR FAR*)&knownPeer[i], sizeof(knownPeer[i]));
					send(connector, sendBuf, strlen(sendBuf), 0);
					closesocket(connector);
				}
				continue;
			}

			if (strcmp(cmd, "connect") == 0)
			{
				if (connectedToTracker)
				{
					continue;
				}

				SOCKADDR_IN addr;

				addr.sin_family = AF_INET;
				addr.sin_port = htons(9000);
				char tmpaddr[32];

				sscanf(buf + offset, "%s", tmpaddr);
				addr.sin_addr.s_addr = inet_addr(tmpaddr);

				connector = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

				int c = connect(connector, (SOCKADDR FAR*)&addr, sizeof(addr));

				if (c < 0)
				{
					printf("Error: %d\n", GetLastError());
				}
				else
				{
					connectedToTracker = true;
					//printf("Peer: %hu\n", listenerPort);
					sprintf(buf, "%hu\n", htons(listenerPort));
					send(connector, buf, strlen(buf), 0);
				}
				continue;
			}

			if (strcmp(cmd, "disconnect") == 0)
			{
				connectedToTracker = false;
				continue;
			}

			if (strcmp(cmd, "upload") == 0)
			{
				char filename[256];
				sscanf(buf + offset, "%s", filename);

				int length = 0;
				int ret;
				char tmpbuf[4096];
				FILE *f = fopen(filename, "rb");
				while (true)
				{
					ret = fread(tmpbuf, 1, 4096, f);
					if (ret == 0)
					{
						break;
					}
					length++;
					if (ret < 4096)
					{
						break;
					}
				}

				char tmpBuf[4096];

				sprintf(tmpBuf, "upload %s %d", filename, length);
				for (int i = 0; i < knownAmount; i++)
				{
					SOCKET connector = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
					connect(connector, (SOCKADDR FAR*)&knownPeer[i], sizeof(knownPeer[i]));
					send(connector, tmpBuf, strlen(tmpBuf), 0);
					closesocket(connector);
				}

				continue;
			}

			if (strcmp(cmd, "getfilename") == 0)
			{

			}

			if (strcmp(cmd, "download") == 0)
			{
				char filename[256];
				int length;
				sscanf(buf + offset, "%s", filename);

				/*bool find = false;
				for (int i = 0; i < fileAmount; i++)
				{
					if (strcmp(fileList[i], filename) == 0)
					{
						length = fileLength[i];
						find = true;
						break;
					}
				}

				if (!find)
				{
					printf("Khong tim thay file\n");
					continue;
				}*/

				/*int *e = new bool[length];
				for (int i = 0; i < length; i++)
				{
					e[i] = i;
				}*/

				char tmpBuf[4096];
				SOCKET connector;
				fd_set recvfds;
				bool complete = false;

				sprintf(tmpBuf, "download %s", filename);

				while (!complete)
					fopen(filename, "wb");
				int p = time(NULL) % knownAmount;
				while (true)
				{
					FILE *f = fopen(filename, "wb");
					int length = 0;
					connector = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
					if (connect(connector, (SOCKADDR FAR*)&knownPeer[p], sizeof(knownPeer[p])) != 0)
					{
						continue;
					}
					send(connector, tmpBuf, strlen(tmpBuf), 0);

					FD_ZERO(&recvfds);
					FD_SET(connector, &recvfds);

					int ret = select(0, &recvfds, NULL, NULL, &timelimit);

					if (ret <= 0)
					{
						if (length > 0)
						{
							fclose(f);
							closesocket(connector);
							complete = true;
						}
					}

					ret = recv(connector, tmpBuf, sizeof(tmpBuf), 0);
					if (ret <= 0)
					{
						if (length > 0)
						{
							fclose(f);
							closesocket(connector);
							complete = true;
						}
					}

					for (int i = 0; i < ret; i++)
					{
						fputc(tmpBuf[i], f);
					}
				}
			}

			continue;
		}

		for (int i = 0; i < knownAmount; i++)
		{
			SOCKET connector = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			connect(connector, (SOCKADDR FAR*)&knownPeer[i], sizeof(knownPeer[i]));
			send(connector, buf, strlen(buf), 0);
			closesocket(connector);
		}
	}
}

int InitializeListener()
{
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	listenerPort = 9001;

	listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	while (true)
	{
		addr.sin_port = htons(listenerPort);
		int ret = bind(listener, (SOCKADDR FAR*)&addr, sizeof(addr)); 
		if (ret == 0)
		{
			break;
		}
		if (listenerPort == 65000)
		{
			return -1;
		}
		listenerPort++;
	}
	listen(listener, 1024);
	return 0;
}

DWORD WINAPI HandleRequest(LPVOID lpParam)
{
	char buf[4096];
	strcpy(buf, (char*)lpParam);
	//((char*)lpParam)[0] = 0;

	SOCKET peer;
	char cmd[4096];
	int offset;

	sscanf(buf, "%d%s%n", &peer, cmd, &offset);
	if (cmd[0] == '\'' || strcmp("chat", cmd) == 0)
	{
		printf("%s", buf);
		return 0;
	}

	if (strcmp(cmd, "download") == 0)
	{
		char filename[4096];
		int length;
		sscanf(buf + offset, "%s", filename);
		FILE *f = fopen(filename, "rb");
		/*if (fseek(f, length * 4096, SEEK_SET) != 0)
		{
			return 0;
		}*/

		char sendBuf[4096];
		while (true)
		{
			printf("ahihi\n");
			int ret = fread(sendBuf, 1, 4096, f);
			if (ret == 0)
			{
				break;
			}
			send(peer, sendBuf, ret, 0);
			if (ret < 4096)
			{
				break;
			}
		}
	}

	if (strcmp(cmd, "upload") == 0)
	{
		char filename[4096];
		int length;
		sscanf(buf + offset, "%s%d", filename, &length);

		strcpy(fileList[fileAmount], filename);
		fileLength[fileAmount] = length;

		fileAmount++;
	}

	printf("%s", buf);
	return 0;
}