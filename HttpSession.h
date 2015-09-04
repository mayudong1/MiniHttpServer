#ifndef __HTTPSESSION_H_
#define __HTTPSESSION_H_

class CHttpSession
{
public:
	CHttpSession();
	~CHttpSession();

	int Start(class CHttpServer* pServer, SOCKET sock, SOCKADDR_IN remoteAddr, string strRootPath);
	int Stop();

private:
	string m_strRootPath;
	SOCKET m_remoteSock;
	SOCKADDR_IN m_remoteAddr;
	class CHttpServer* m_pServer;

	HANDLE m_hExitEvent;
	HANDLE m_hWorkThread;
	static unsigned int __stdcall WorkThread(void* pParam);

	int ParseBuffer(char* pBuffer, int nLen);
	int Send(char* pBuffer, int nLen);
};

#endif