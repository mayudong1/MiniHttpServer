#include "HTTPServer.h"


CHTTPServer::CHTTPServer(void)
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	m_sockListener = INVALID_SOCKET;
	m_strRootPath = "";
	memset(&m_stLocalAddr, 0, sizeof(m_stLocalAddr));
}


CHTTPServer::~CHTTPServer(void)
{
	WSACleanup();
}

int CHTTPServer::Start(const unsigned short usPort, const char* szRootPath, int nMaxClient)
{
	int nRet = 0;

	m_strRootPath = szRootPath;

	m_sockListener = socket(AF_INET, SOCK_STREAM, 0);
	if(INVALID_SOCKET == m_sockListener)
	{
		printf("Create socket failed. ErrCode = %d\n", WSAGetLastError());
		return -1;
	}

	m_stLocalAddr.sin_family = AF_INET;
	m_stLocalAddr.sin_addr.s_addr = INADDR_ANY;
	m_stLocalAddr.sin_port = htons(usPort);
	nRet = bind(m_sockListener, (SOCKADDR*)&m_stLocalAddr, sizeof(m_stLocalAddr));
	if(SOCKET_ERROR == nRet)
	{
		printf("bind() failed, ErrCode = %d\n", WSAGetLastError());
		closesocket(m_sockListener);
		return -1;
	}

	int timeout = 1000;
	nRet = setsockopt(m_sockListener, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
	if(SOCKET_ERROR == nRet)
	{
		printf("Set recieve timeout failed, ErrCode = %d\n", WSAGetLastError());
		closesocket(m_sockListener);
		return -1;
	}

	nRet = listen(m_sockListener, 20);
	if(SOCKET_ERROR == nRet)
	{
		printf("listen() failed, ErrCode = %d\n", WSAGetLastError());
		closesocket(m_sockListener);
		return -1;
	}
	return 0;
}

int CHTTPServer::Stop()
{
	closesocket(m_sockListener);
	m_sockListener = INVALID_SOCKET;
	return 0;
}
