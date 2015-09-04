#include "HttpHeader.h"
#include "HttpServer.h"
#include "HttpSession.h"
#include "HttpRequestParse.h"

#define MAX_BUF_SIZE (4096)

CHttpSession::CHttpSession()
{
	m_strRootPath = "";
	m_remoteSock = INVALID_SOCKET;
	memset(&m_remoteAddr, 0, sizeof(m_remoteAddr));
	m_pServer = NULL;

	m_hExitEvent = NULL;
	m_hWorkThread = NULL;
}

CHttpSession::~CHttpSession()
{
	Stop();
}

int CHttpSession::Start(class CHttpServer* pServer, SOCKET sock, SOCKADDR_IN remoteAddr, string strRootPath)
{
	m_pServer = pServer;
	m_strRootPath = strRootPath;
	m_remoteSock = sock;
	m_remoteAddr = remoteAddr;

	m_hExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hWorkThread = (HANDLE)_beginthreadex(NULL, 0, WorkThread, this, 0, NULL);

	return 0;
}

int CHttpSession::Stop()
{
	if(m_remoteSock != INVALID_SOCKET)
	{
		closesocket(m_remoteSock);
		m_remoteSock = INVALID_SOCKET;
	}

	if(m_hExitEvent)
	{
		SetEvent(m_hExitEvent);
		if(WAIT_TIMEOUT == WaitForSingleObject(m_hWorkThread, 3000))
		{
			TerminateThread(m_hWorkThread, -1);
		}
		CloseHandle(m_hExitEvent);
		CloseHandle(m_hWorkThread);
		m_hExitEvent = NULL;
		m_hWorkThread = NULL;
	}

	return 0;
}

unsigned int __stdcall CHttpSession::WorkThread(void* pParam)
{
	CHttpSession* pObj = (CHttpSession*)pParam;
	while(WAIT_TIMEOUT == WaitForSingleObject(pObj->m_hExitEvent, 0))
	{
		fd_set read_fds;
		struct timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		SOCKET sock = pObj->m_remoteSock;
		FD_ZERO(&read_fds);
		FD_SET(sock, &read_fds);
		int nRet = select(0, &read_fds, NULL, NULL, &timeout);		
		if(nRet == 0)
		{			
			continue;
		}
		else if(nRet > 0)
		{
			char buffer[MAX_BUF_SIZE] = {0};
			int nLen = recv(sock, buffer, MAX_BUF_SIZE, 0);
			if(nLen <= 0)
			{
				pObj->m_pServer->ExitClient(sock);
				break;
			}

			int nRet = pObj->ParseBuffer(buffer, nLen);
			if(nRet == 0)
			{
				//pObj->m_pServer->ExitClient(sock);
				//break;		
			}
		}
		else
		{
			pObj->m_pServer->ExitClient(sock);
			break;
		}
	}
	return 0;
}

int CHttpSession::ParseBuffer(char* pBuffer, int nLen)
{
	printf("recv: %s\n", pBuffer);
	Send(pBuffer, nLen);
	return 0;
}

int CHttpSession::Send(char* pBuffer, int nLen)
{
	int nLeftLen = nLen;
	char* pTmpBuffer = pBuffer;
	while(nLeftLen > 0)
	{
		int nToSend = min(1316, nLeftLen);
		fd_set write_fds;
		struct timeval timeout;
		FD_ZERO(&write_fds);
		FD_SET(m_remoteSock, &write_fds);

		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		int nRet = select(0, NULL, &write_fds, NULL, &timeout);
		if(nRet > 0)
		{
			int nSent = 0;
			if (FD_ISSET(m_remoteSock, &write_fds))
			{
				nSent = send(m_remoteSock, (char*)pTmpBuffer, (int)nToSend, 0);
				if(nSent < 0)
				{
					return -1;
				}
			}	
			nLeftLen -= nSent;
			pTmpBuffer += nSent;
		}
		else if(nRet < 0)
		{
			return -1;
		}
		else
		{
			continue;
		}	
	}

	return 0;
}