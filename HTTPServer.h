#ifndef __HTTPSERVER_H_
#define __HTTPSERVER_H_

#include "HTTPHeader.h"

#define MAX_CLIENT 1000

typedef map<SOCKET, class CHttpSession*> HttpSessionMap;

class CHttpServer
{
	friend class CHttpSession;
public:
	CHttpServer(void);
	~CHttpServer(void);

	int Start(const unsigned short usPort, const char* szRootPath = "", int nMaxClient = MAX_CLIENT);
	int Stop();

private:
	string m_strRootPath;
	SOCKET m_sockListener;
	SOCKADDR_IN m_stLocalAddr;

	CRITICAL_SECTION m_csForSessionMap;
	HttpSessionMap m_mapSession;

	HANDLE m_hAcceptEvent;
	HANDLE m_hAcceptThread;
	static unsigned int __stdcall ListenThread(void* pParam);

	int AcceptClient();

	int AddClient(SOCKET sock, SOCKADDR_IN remoteAddr);
	int DelClient(SOCKET sock);

	CRITICAL_SECTION m_csForExitClient;
	queue<SOCKET> m_queueExitClient;
	int ExitClient(SOCKET sock);
};


#endif