#include <stdio.h>
#include "HttpServer.h"

int main(int argc, char** argv)
{
	printf("this is a test\n");

	unsigned short usPort = 0;
	if(argc >= 2)
	{
		usPort = atoi(argv[1]);
	}
	else
	{
		usPort = 8080;
	}

	CHttpServer server;
	int nRet = server.Start(8080);
	if(nRet == 0)
	{
		printf("Start server success on port : %d.\n", usPort);
	}
	else
	{
		printf("Start server failed.\n");
		return -1;
	}
	getchar();
	server.Stop();
	return 0;
}