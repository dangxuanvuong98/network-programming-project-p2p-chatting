
#include "stdafx.h"

LISTENER listener;

USER userList;
set<FILE_INFO> fileList;

CONNECTION onlinePeer;
int onlinePeerAmount;

CRITICAL_SECTION criticalSection;