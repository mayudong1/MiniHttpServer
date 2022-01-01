#include <stdio.h>
#include "HttpServer.h"
#include <string.h>

int main(int argc, char** argv)
{
	char wwwroot[256];
	strcpy(wwwroot, "/Users/mayudong/Movies");
	unsigned short usPort = 0;
	if(argc >= 2)
	{
		usPort = atoi(argv[1]);
	}
	else
	{
		usPort = 8080;
	}
	if(argc >= 3)
	{
		strcpy(wwwroot, argv[2]);
	}

	CHttpServer server;
	int nRet = server.Start(usPort, wwwroot);
	if(nRet == 0)
	{
		printf("Start server success on port : %d.\n", usPort);
	}
	else
	{
		printf("Start server failed.\n");
		return -1;
	}
	while(1)
	{
		char c = getchar();	
		if(c == 'q'){
			break;
		}else if(c == 'p'){
			server.Pause();
		}else if(c == 'r'){
			server.Resume();
		}
	}
	
	server.Stop();
	return 0;
}
