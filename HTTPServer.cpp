#include "HttpServer.h"
#include "HttpSession.h"


CHttpServer::CHttpServer(void)
{
	m_sockListener = INVALID_SOCKET;
	m_strRootPath = "";
	memset(&m_stLocalAddr, 0, sizeof(m_stLocalAddr));

	m_bExit = false;

	pthread_mutex_init(&m_csForSessionMap, NULL);
	pthread_mutex_init(&m_csForExitClient, NULL);

}


CHttpServer::~CHttpServer(void)
{
	pthread_mutex_destroy(&m_csForSessionMap);
	pthread_mutex_destroy(&m_csForExitClient);
}

int CHttpServer::Start(const unsigned short usPort, const char* szRootPath, int nMaxClient)
{
	int nRet = 0;

	m_strRootPath = szRootPath;

	m_sockListener = socket(AF_INET, SOCK_STREAM, 0);
	if(INVALID_SOCKET == m_sockListener)
	{
		printf("Create socket failed. ErrCode = %d\n", errno);
		return -1;
	}

	m_stLocalAddr.sin_family = AF_INET;
	m_stLocalAddr.sin_addr.s_addr = INADDR_ANY;
	m_stLocalAddr.sin_port = htons(usPort);
	nRet = bind(m_sockListener, (SOCKADDR*)&m_stLocalAddr, sizeof(m_stLocalAddr));
	if(SOCKET_ERROR == nRet)
	{
		printf("bind() failed, ErrCode = %d\n", errno);
		close(m_sockListener);
		return -1;
	}

	nRet = listen(m_sockListener, 20);
	if(SOCKET_ERROR == nRet)
	{
		printf("listen() failed, ErrCode = %d\n", errno);
		close(m_sockListener);
		return -1;
	}

	pthread_create(&m_hAcceptThread, NULL, ListenThread, this);
	return 0;
}

int CHttpServer::Stop()
{
	m_bExit = true;

	for(HttpSessionMap::iterator iter = m_mapSession.begin(); iter != m_mapSession.end(); iter++)
	{
		iter->second->Stop();
		delete iter->second;
	}
	m_mapSession.clear();

	if(m_sockListener == INVALID_SOCKET)
	{
		close(m_sockListener);
		m_sockListener = INVALID_SOCKET;	
	}

	return 0;
}

void* CHttpServer::ListenThread(void* pParam)
{
	CHttpServer* pObj = (CHttpServer*)pParam;	
	while(!pObj->m_bExit)
	{
		{
			CAutoLock locker(&pObj->m_csForExitClient);
			while(!pObj->m_queueExitClient.empty())
			{
				SOCKET sock = pObj->m_queueExitClient.front();
				pObj->m_queueExitClient.pop();
				pObj->DelClient(sock);
			}
		}

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
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	int maxfd = m_sockListener + 1;
	int nRet = select(maxfd, &read_fds, NULL, NULL, &timeout);
	if(nRet <= 0)
	{		
		return 0;
	}

	if(FD_ISSET(m_sockListener, &read_fds))
	{
		socklen_t len = sizeof(SOCKADDR_IN);
		SOCKADDR_IN remoteAddr;
		SOCKET newSock = accept(m_sockListener, (struct sockaddr*)&remoteAddr, &len);
		if(INVALID_SOCKET == newSock)
		{
			printf("accept() failed, ErrCode = %d\n", errno);
			return -1;
		}
		else
		{			
			AddClient(newSock, remoteAddr);
		}
	}
	return 0;
}

int CHttpServer::AddClient(SOCKET sock, SOCKADDR_IN remoteAddr)
{
	CAutoLock locker(&m_csForSessionMap);
	CHttpSession* pHttpSession = new CHttpSession();
	int nRet = pHttpSession->Start(this, sock, remoteAddr, m_strRootPath);
	if(nRet != 0)
	{
		printf("session start failed\n");
		delete pHttpSession;
		return -1;
	}
	m_mapSession[sock] = pHttpSession;
	printf("accept new client, addr = %s, port = %d\n", inet_ntoa(remoteAddr.sin_addr), ntohs(remoteAddr.sin_port));
	return 0;
}

int CHttpServer::DelClient(SOCKET sock)
{
	CAutoLock locker(&m_csForSessionMap);
	HttpSessionMap::iterator iter = m_mapSession.find(sock);
	if(iter != m_mapSession.end())
	{
		delete iter->second;
		m_mapSession.erase(iter);
		printf("delete client\n");
		return 0;
	}
	return -1;
}

int CHttpServer::ExitClient(SOCKET sock)
{
	CAutoLock locker(&m_csForExitClient);
	m_queueExitClient.push(sock);
	return 0;
}