#pragma once

#include "HTTPHeader.h"

#define MAX_CLIENT 1000

class CHTTPServer
{
public:
	CHTTPServer(void);
	~CHTTPServer(void);

	int Start(const unsigned short usPort, const char* szRootPath = "", int nMaxClient = MAX_CLIENT);
	int Stop();

private:
	string m_strRootPath;
	SOCKET m_sockListener;
	SOCKADDR_IN m_stLocalAddr;
};

