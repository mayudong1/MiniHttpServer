#include "HttpHeader.h"
#include "HttpServer.h"
#include "HttpSession.h"


#define MAX_BUF_SIZE (4096)

CHttpSession::CHttpSession()
{
	m_pHttpParse = NULL;
	m_strRootPath = ".";
	m_remoteSock = INVALID_SOCKET;
	memset(&m_remoteAddr, 0, sizeof(m_remoteAddr));
	m_pServer = NULL;
	m_bExit = false;
}

CHttpSession::~CHttpSession()
{
	Stop();
}

int CHttpSession::Start(class CHttpServer* pServer, SOCKET sock, SOCKADDR_IN remoteAddr, string strRootPath)
{
	m_pHttpParse = new CHttpRequestParse();
	m_pServer = pServer;
	m_strRootPath = strRootPath;
	m_remoteSock = sock;
	m_remoteAddr = remoteAddr;

	pthread_create(&m_hWorkThread, NULL, WorkThread, this);

	return 0;
}

int CHttpSession::Stop()
{
	if(m_remoteSock != INVALID_SOCKET)
	{
		close(m_remoteSock);
		m_remoteSock = INVALID_SOCKET;
	}

	if(m_pHttpParse)
	{
		delete m_pHttpParse;
		m_pHttpParse = NULL;
	}

	return 0;
}

void* CHttpSession::WorkThread(void* pParam)
{
	CHttpSession* pObj = (CHttpSession*)pParam;
	while(!pObj->m_bExit)
	{
		fd_set read_fds;
		struct timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		SOCKET sock = pObj->m_remoteSock;
		FD_ZERO(&read_fds);
		FD_SET(sock, &read_fds);
		int maxfd = sock + 1;
		int nRet = select(maxfd, &read_fds, NULL, NULL, &timeout);		
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
				printf("client disconnect\n");
				pObj->m_pServer->ExitClient(sock);
				break;
			}

			int nRet = pObj->ParseBuffer(buffer, nLen);
			if(nRet == 0)
			{
				printf("disconnect client\n");
				pObj->m_pServer->ExitClient(sock);
				break;		
			}
		}
		else
		{
			printf("disconnect client\n");
			pObj->m_pServer->ExitClient(sock);
			break;
		}
	}
	return NULL;
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
		int maxfd = m_remoteSock+1;
		int nRet = select(maxfd, NULL, &write_fds, NULL, &timeout);
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

int CHttpSession::SendHttpError(int nErrorCode)
{
	const char *not_found =
		"HTTP/1.1 404 Not Found\r\n"		
		"Content-Type: text/html\r\n"
		"Content-Length: 40\r\n"
		"\r\n"
		"<HTML><BODY>File not found</BODY></HTML>\r\n";

	const char *bad_request =
		"HTTP/1.1 400 Bad Request\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: 39\r\n"
		"\r\n"
		"<h1>Bad Request (Invalid Hostname)</h1>\r\n";

	const char* nul_request = 
		"HTTP/1.1 204 OK"
		"Content-Type: text/html"
		"Content-Length: 0"
		"\r\n";

	switch(nErrorCode)
	{
	case 404:
		Send((char*)not_found, (int)strlen(not_found));
		break;
	case 400:
		Send((char*)bad_request, (int)strlen(bad_request));
		break;
	case 204:
		Send((char*)nul_request, (int)strlen(nul_request));
		break;
	default:
		Send((char*)not_found, (int)strlen(not_found));
		break;
	}
	return 0;
}

int CHttpSession::SendHttpString(string strData)
{
	int nRetCode = 200;
	int nContLen = strData.length();

	char resp_head[MAX_BUF_SIZE];
	sprintf(resp_head,
		"HTTP/1.1 %d OK\r\n"
		"Accept-Ranges: bytes\r\n"
		"Content-Length:%u\r\n"		
		"Content-Type: text/plain\r\n"
		"\r\n", nRetCode, nContLen);

	Send(resp_head, strlen(resp_head));
	Send((char*)strData.c_str(), strData.length());
	return 0;
}

int CHttpSession::ProcessFileRequest(string strFileName, int nFileStart, int nFileStop)
{
	strFileName = m_strRootPath + "/" + strFileName;
	
	FILE* pFile = fopen(strFileName.c_str(), "rb");
	if(pFile == NULL)
	{
		printf("open file failed, filename = %s\n", strFileName.c_str());
		SendHttpError(404);
		return -1;
	}	

	fseek(pFile, 0, SEEK_END);
	int nFileSize = ftell(pFile);
	int nFileStartPos = min(nFileStart, max(nFileSize, 0));
	fseek(pFile, nFileStartPos, SEEK_SET);
	
	int nRetCode = 200;
	string strFileExt = "";
	string strContentType = "application/octet-stream";
	size_t nPos = strFileName.find_last_of(".");
	if(nPos != string::npos)
	{
		strFileExt = strFileName.substr(nPos+1, strFileName.length()-nPos);		
	}
	string strdest;
	strdest.resize(strFileExt.size());
	transform(strFileExt.begin(), strFileExt.end(), strdest.begin(), ::tolower);
	if(m_pHttpParse)
	{
		strContentType = m_pHttpParse->GetContentType(strFileExt);
	}

	char resp_head[MAX_BUF_SIZE];
	sprintf(resp_head,
		"HTTP/1.1 %d OK\r\n"
		"Accept-Ranges: bytes\r\n"
		"Content-Length:%d\r\n"
		"Content-Range: bytes %d-%d/%d\r\n"
		"Content-Type: %s\r\n"		
		"\r\n", 
		nRetCode, 
		min(nFileSize-nFileStartPos, max(nFileSize, 0)), 
		nFileStartPos, nFileSize-1, nFileSize,
		strContentType.c_str()
		);
	Send(resp_head, strlen(resp_head));
	printf(resp_head);
	
	char buffer[4096];
	int readed = 0;
	while(true)
	{
		readed = (int)fread(buffer, 1, sizeof(buffer), pFile);
		if(readed <= 0)
		{
			break;
		}

		if(Send(buffer, readed) != 0)
		{
			break;
		}
	}
	fclose(pFile);
	return 0;
}

//返回值 0:断开连接   1:保持连接
int CHttpSession::ParseBuffer(char* pBuffer, int nLen)
{
	printf(pBuffer);

	if(m_pHttpParse == NULL)
	{
		SendHttpError(400);
		return 0;
	}

	//nRet=0正常结束 <0有错误 >0http头未结束
	int nRet = m_pHttpParse->Input(pBuffer, m_stRequestInfo);
	if(nRet < 0)
	{
		SendHttpError(400);		
		return 0;
	}
	else if(nRet > 0)
	{
		return 1;
	}

	if(m_stRequestInfo.method == "GET")
	{
		string szURLName;
		if(m_stRequestInfo.url != "/" && m_stRequestInfo.url != "")
		{
			szURLName = m_stRequestInfo.url.substr(1, m_stRequestInfo.url.length());

			int nFileStart = 0;
			int nFileStop = 0;
			http_header::iterator it_range = m_stRequestInfo.http_headers.find("Range");
			if(it_range != m_stRequestInfo.http_headers.end())
			{
				string szRangeContext = it_range->second;
				size_t nPos0 = szRangeContext.find('=');
				size_t nPos1 = szRangeContext.find('-');
				if(nPos1 != string::npos && nPos1 != string::npos)
				{					
					string szStart = szRangeContext.substr(nPos0+1, nPos1-nPos0-1);
					string szStop = szRangeContext.substr(nPos1+1, szRangeContext.length());
					nFileStart = atoi(szStart.data());
					nFileStop = atoi(szStop.data());
				}
			}
			ProcessFileRequest(szURLName, nFileStart, nFileStop);
		}
	}
	else if(m_stRequestInfo.method == "POST")
	{		
		SendHttpString(m_stRequestInfo.body);
	}

	bool bKeepAlive = false;
	http_header::iterator it_connection = m_stRequestInfo.http_headers.find("Connection");
	if(it_connection != m_stRequestInfo.http_headers.end())
	{
		if(it_connection->second == "keep-alive")
		{
			bKeepAlive = true;
		}
	}	
	
	m_stRequestInfo.body = "";
	m_stRequestInfo.http_headers.clear();
	m_stRequestInfo.method = "";
	m_stRequestInfo.url = "";
	m_stRequestInfo.version = "";

	return bKeepAlive ? 1 : 0;
}

