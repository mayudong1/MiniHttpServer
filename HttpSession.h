#ifndef __HTTPSESSION_H_
#define __HTTPSESSION_H_

#include "HttpRequestParse.h"

class CHttpSession
{
public:
	CHttpSession();
	~CHttpSession();

	int Start(class CHttpServer* pServer, SOCKET sock, SOCKADDR_IN remoteAddr, string strRootPath);
	int Stop();
	int Pause();
	int Resume();

private:
	class CHttpRequestParse* m_pHttpParse;

	RequestInfo m_stRequestInfo;
	string m_strRootPath;
	SOCKET m_remoteSock;
	SOCKADDR_IN m_remoteAddr;
	class CHttpServer* m_pServer;

	volatile bool m_bPaused;
	volatile bool m_bExit;
	pthread_t m_hWorkThread;
	static void* WorkThread(void* pParam);

	int ParseBuffer(char* pBuffer, int nLen);
	int Send(char* pBuffer, int nLen);
	int SendHttpError(int nErrorCode);
	int SendHttpString(string strData);

	int ProcessFileRequest(string strFileName, int nFileStart, int nFileStop);
};

#endif