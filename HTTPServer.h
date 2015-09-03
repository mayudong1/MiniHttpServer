#pragma once

#define MAX_CLIENT 1000
class CHTTPServer
{
public:
	CHTTPServer(void);
	~CHTTPServer(void);

	int Start(const unsigned short usPort, const char* szRootPath = "", int nMaxClient = MAX_CLIENT);
	int Stop();
};

