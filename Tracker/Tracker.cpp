#include "stdafx.h"

using namespace std;

int main()
{
	InitializeCriticalSection(&criticalSection);

	InitListener(listener);
	CreateThread(0, 0, WaitForNewConnection, NULL, 0, 0);
	CreateThread(0, 0, WaitForRequest, NULL, 0, 0);

	DeleteCriticalSection(&criticalSection);
    return 0;
}