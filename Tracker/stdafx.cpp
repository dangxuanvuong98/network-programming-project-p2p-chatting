
#include "stdafx.h"

bool operator<(FILE_INFO lhs, FILE_INFO rhs)
{
	return (strncmp(lhs.name, rhs.name, 32) < 0);
}

LISTENER listener;

USER userList;
set<FILE_INFO> fileList;

CONNECTION onlinePeer;
int onlinePeerAmount;

CRITICAL_SECTION criticalSection;