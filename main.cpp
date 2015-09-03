#include <stdio.h>
#include "HTTPServer.h"

int main()
{
	printf("this is a test\n");
	CHTTPServer server;
	server.Start(8080);
	getchar();
	server.Stop();
	return 0;
}