#include "HttpHeader.h"
#include "HttpRequestParse.h"


static void trim(string& str)
{
	string::size_type pos1 = str.find_first_not_of(' ');
	string::size_type pos2 = str.find_last_not_of(' ');
	str = str.substr(pos1 == string::npos ? 0 : pos1, 
		pos2 == string::npos ? str.length() - 1 : pos2 - pos1 + 1);
}


CHttpRequestParse::CHttpRequestParse()
{
	m_bHasbody = false;
	m_nContentLen = 0;
	init_map_file_type();
}

CHttpRequestParse::~CHttpRequestParse()
{

}

int CHttpRequestParse::Reset()
{
	m_strRequestHeader = "";
	m_bHasbody = false;
	m_nContentLen = 0;
	return 0;
}

//返回值 -1：有错误 
//		  0: 正常解析结束
//		  1: http头还不完整
int CHttpRequestParse::Input(string strInput, RequestInfo& stRequestInfo)
{
	m_strRequestHeader += strInput;
	size_t nPos = m_strRequestHeader.find("\r\n\r\n");
	if(nPos != string::npos && !m_bHasbody)
	{
		int nRet = parse_http_request(m_strRequestHeader, stRequestInfo);
		if(nRet != 0)
		{
			Reset();
			return -1;
		}
		int nContLen = get_content_length(stRequestInfo);
		if(nContLen > 0)
		{
			m_bHasbody = true;
			m_nContentLen = nContLen;		
		}
		else
		{
			Reset();
			return 0;
		}
	}
	if(m_bHasbody && m_strRequestHeader.length() == nPos+m_nContentLen+4)
	{		
		stRequestInfo.body = m_strRequestHeader.substr(nPos+4, m_nContentLen);		
		Reset();
		return 0;
	}
	Reset();
	return 0;
}

int CHttpRequestParse::parse_http_request(string header, RequestInfo &stRequestInfo)
{
	string sub_header = header;
	trim(sub_header);
	if(sub_header == "")
	{
		return -1;
	}

	stRequestInfo.method = get_method(sub_header);
	stRequestInfo.url = get_url(sub_header);
	stRequestInfo.version = get_http_version(sub_header);
	if(!is_valid_http_method(stRequestInfo.method))
	{
		return -2;
	}

	if(stRequestInfo.url[0] != '/')
	{
		return -3;
	}

	if(stRequestInfo.version!="1.0" && stRequestInfo.version!="1.1")
	{
		return -4;
	}

	stRequestInfo.http_headers = parse_http_headers(sub_header);

	return 0;
}

string CHttpRequestParse::get_method(string &header)
{
	string tmp_header = header;
	string method = "";

	size_t pos_method = tmp_header.find_first_of(' ');
	if(pos_method != string::npos)
	{
		method = tmp_header.substr(0, pos_method);
		header = tmp_header.substr(pos_method+1, tmp_header.length());	
	}

	return method;
}

string CHttpRequestParse::get_url(string &header)
{
	string tmp_header = header;
	string url = "";

	size_t pos_method = tmp_header.find_first_of(' ');
	if(pos_method != string::npos)
	{
		url = tmp_header.substr(0, pos_method);
		header = tmp_header.substr(pos_method+1, tmp_header.length());	
	}

	return url;
}

string CHttpRequestParse::get_http_version(string &header)
{
	string tmp_header = header;
	string version;

	size_t pos_method = tmp_header.find_first_of("\r\n");
	if(pos_method != string::npos)
	{
		version = tmp_header.substr(0, pos_method);
		header = tmp_header.substr(pos_method+2, tmp_header.length());	
	}
	
	if(version.length() <= 5)
	{
		version = "";
		return version;
	}
	version = version.substr(5, version.length());

	return version;
}

bool CHttpRequestParse::is_valid_http_method(string method)
{
	if(method!="GET" && method!="POST" &&
		method!="HEAD" && method!="CONNECT" &&
		method!="PUT" && method!="DELETE")
	{
		return false;
	}
	return true;
}

http_header CHttpRequestParse::parse_http_headers(string &header)
{
	string tmp_header = header;
	http_header http_headers;

	while(TRUE)
	{
		if(tmp_header.length() <= 0)
		{
			break;
		}

		string name;
		string value;

		size_t pos_name = tmp_header.find_first_of(':');
		if((int)pos_name < 0)
		{
			break;
		}

		name = tmp_header.substr(0, pos_name);
		tmp_header = tmp_header.substr(pos_name+2, tmp_header.length());

		size_t pos_value = tmp_header.find_first_of('\r');
		if((int)pos_value < 0)
		{
			break;
		}
		value = tmp_header.substr(0, pos_value);
		tmp_header = tmp_header.substr(pos_value+2, tmp_header.length());

		http_headers[name] = value;
	}

	return http_headers;
}

int CHttpRequestParse::get_content_length(RequestInfo &stRequestInfo)
{
	string szContLen = "";
	for(http_header::iterator it=stRequestInfo.http_headers.begin(); it!=stRequestInfo.http_headers.end(); it++)
	{
		//if(it->first == "Content-Length")
		if(stricmp(it->first.c_str(), "Content-Length") == 0)
		{
			szContLen = it->second;
			break;
		}
	}

	int nContLen = atoi(szContLen.c_str());
	return nContLen;
}

void CHttpRequestParse::init_map_file_type()
{
	m_mapFileType.clear();
	m_mapFileType.insert(make_pair("jpg", "image/jpeg"));
	m_mapFileType.insert(make_pair("xml", "text/xml"));
	m_mapFileType.insert(make_pair("cpp", "text/plain"));
	m_mapFileType.insert(make_pair("txt", "text/plain"));
}

string CHttpRequestParse::GetContentType(string strFileExtName)
{
	string strContentType = "application/octet-stream";
	map<string, string>::iterator iter = m_mapFileType.find(strFileExtName);
	if(iter != m_mapFileType.end())
	{
		strContentType = iter->second;
	}
	return strContentType;
}