#include "HTTPServer.h"


CHttpServer::CHttpServer(void)
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	m_sockListener = INVALID_SOCKET;
	m_strRootPath = "";
	memset(&m_stLocalAddr, 0, sizeof(m_stLocalAddr));

	m_hAcceptEvent = NULL;
	m_hAcceptThread = NULL;
}


CHttpServer::~CHttpServer(void)
{
	WSACleanup();
}

int CHttpServer::Start(const unsigned short usPort, const char* szRootPath, int nMaxClient)
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

	m_hAcceptEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hAcceptThread = (HANDLE)_beginthreadex(NULL, 0, ListenThread, this, 0, NULL);
	return 0;
}

int CHttpServer::Stop()
{
	closesocket(m_sockListener);
	m_sockListener = INVALID_SOCKET;
	return 0;
}

unsigned int __stdcall CHttpServer::ListenThread(void* pParam)
{
	CHttpServer* pObj = (CHttpServer*)pParam;
	while(WAIT_TIMEOUT == WaitForSingleObject(pObj->m_hAcceptEvent, 0))
	{
		pObj->AcceptClient();
	}
	return 0;
}

int CHttpServer::AcceptClient()
{
	fd_set read_fds;
	struct timeval timeout;

	FD_ZERO(&read_fds);
	FD_SET(m_sockListener, &read_fds);
	timeout.tv_sec = 0;
	timeout.tv_usec = 100000;

	int nRet = select(0, &read_fds, NULL, NULL, &timeout);
	if(nRet <= 0)
	{
		return 0;
	}

	if(FD_ISSET(m_sockListener, &read_fds))
	{
		int len = sizeof(SOCKADDR_IN);
		SOCKADDR_IN remoteAddr;
		SOCKET newSock = accept(m_sockListener, (SOCKADDR*)&remoteAddr, &len);
		if(INVALID_SOCKET == newSock)
		{
			printf("accept() failed, ErrCode = %d\n", WSAGetLastError());
			return -1;
		}
		else
		{
			printf("accept new client, addr = %s, port = %d\n", inet_ntoa(remoteAddr.sin_addr), ntohs(remoteAddr.sin_port));
		}
	}
	return 0;
}