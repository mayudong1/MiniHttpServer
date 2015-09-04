#ifndef __HTTPSERVER_H_
#define __HTTPSERVER_H_

#include "HTTPHeader.h"

#define MAX_CLIENT 1000

class CHttpServer
{
public:
	CHttpServer(void);
	~CHttpServer(void);

	int Start(const unsigned short usPort, const char* szRootPath = "", int nMaxClient = MAX_CLIENT);
	int Stop();

private:
	string m_strRootPath;
	SOCKET m_sockListener;
	SOCKADDR_IN m_stLocalAddr;

	HANDLE m_hAcceptEvent;
	HANDLE m_hAcceptThread;
	static unsigned int __stdcall ListenThread(void* pParam);

	int AcceptClient();
};


#endif