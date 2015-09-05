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

	int Input(string strInput, RequestInfo& stRequestInfo);
	string GetContentType(string strFileExtName);
private:
	int Reset();
	int parse_http_request(string header, RequestInfo &stRequestInfo);
	string get_method(string &header);
	string get_url(string &header);
	string get_http_version(string &header);
	bool is_valid_http_method(string method);
	http_header parse_http_headers(string &header);
	int get_content_length(RequestInfo &stRequestInfo);
private:
	string m_strRequestHeader;
	bool m_bHasbody;
	int m_nContentLen;

	void init_map_file_type();
	map<string, string> m_mapFileType;
};


#endif