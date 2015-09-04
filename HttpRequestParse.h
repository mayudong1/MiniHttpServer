#ifndef __HTTP_REQUEST_H_
#define __HTTP_REQUEST_H_

typedef map<string, string> http_header;

typedef struct tagRequestInfo
{
	string method;
	string url;
	string version;
	http_header http_headers;
	char* body;
	tagRequestInfo()
	{
		body = NULL;
	}
}RequestInfo;

class CHttpRequestParse
{
public:
	CHttpRequestParse();
	~CHttpRequestParse();
};


#endif