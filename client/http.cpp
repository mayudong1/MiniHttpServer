#include "http.h"

using namespace std;

HttpClient::HttpClient():_pos(0), _port(80), _socketfd(0), _buffer(NULL)
{
	_content_length = 0;
	_recv_length = 0;
	_read_pos = 0;
	_recving = false;
}
HttpClient::~HttpClient()
{
	if(_readThread.joinable())
	{
		_readThread.join();
	}
	if(_buffer)
	{
		delete _buffer;
		_buffer = NULL;
	}
}

void HttpClient::Test()
{
	std::cout << "http client test" << std::endl;
}

int HttpClient::ParseUrl(const std::string url)
{
	_url = url;
	string tmp = _url;
	size_t pos = tmp.find("http://");
	if(pos == string::npos)
	{
		cout << "url invalid" << endl;
		return -1;
	}
	tmp = tmp.substr(pos+7);
	
	pos = tmp.find(':');
	size_t pos2 = tmp.find('/');
	if(pos == string::npos)
	{
		_port = 80;
		pos = tmp.find('/');
		_fileName = tmp.substr(pos2+1);
	}
	else
	{
		if(pos2 != string::npos)
		{
			_port = atoi(tmp.substr(pos+1, pos2-pos-1).c_str());
			_fileName = tmp.substr(pos2+1);
		}
	}
	_hostName = tmp.substr(0, pos);
	struct hostent *h;
	if((h=gethostbyname(_hostName.c_str()))==NULL)
	{
		return -1;
	}
	_hostIp = inet_ntoa(*((struct in_addr *)h->h_addr));


	cout << "host = " << _hostName << endl;
	cout << "port = " << _port << endl;
	cout << "file = " << _fileName << endl;
	return 0;
}

int HttpClient::Connect()
{
	struct sockaddr_in sockaddr;

	_socketfd = socket(AF_INET,SOCK_STREAM,0);
	memset(&sockaddr,0,sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(_port);
	inet_pton(AF_INET, _hostIp.c_str(), &sockaddr.sin_addr);
	if((connect(_socketfd,(struct sockaddr*)&sockaddr,sizeof(sockaddr))) < 0 )
	{
		std::cout << "connect failed" << std::endl;
		return -1;
	}
	cout << "connect success." << endl;
	return 0;
}

void ReadThread(void* obj)
{
	HttpClient* client = (HttpClient*)obj;
	client->ReadThread();
	return;
}

void HttpClient::ReadThread()
{
	while(1)
	{
		if(_buffer->RemainSize() < 1024)
		{
			usleep(100000);
			cout << "sleep, RemainSize = " << _buffer->RemainSize() << endl;
			continue;
		}

		char tmp[1024];
		memset(tmp, 0, 1024);
		int num = recv(_socketfd, tmp, 1024, 0);
		if(num > 0)
		{
			_recv_length += num;
			_buffer->Put(tmp, num);
			if(_recv_length >= _content_length)
			{
				break;
			}
		}
		else
		{
			break;
		}
	}
	_recving = false;
	close(_socketfd);

}

int HttpClient::ReadHeader()
{
	int ret = 0;
	char request[1024];
	sprintf(request,"GET /%s HTTP/1.1\r\n"
					"Range: bytes=0-\r\n"
					"Host:%s\r\n\r\n",
					_fileName.c_str(), _hostName.c_str());
	string http_request = request;
	cout << request << endl;

	ret = send(_socketfd, http_request.c_str(), http_request.length(), 0);
	if(ret == -1)
	{
		cout << "send http request failed" << endl;
		return ret;
	}

	string recvData;
	char tmp[1024+1];
	bool bGotHeader = false;
	unsigned long long recv_length = 0;
	unsigned long long content_length = 0;
	size_t header_length = 0;
	while(1){
		memset(tmp, 0, 1024+1);
		int num = recv(_socketfd, tmp, 1024, 0);
		if(num > 0)
		{
			recvData.append(tmp, num);
			if(!bGotHeader)
			{
				size_t nPos = recvData.find("\r\n\r\n");
				if(nPos != string::npos && !bGotHeader)
				{
					header_length = nPos + 4;
					bGotHeader = true;

					string retcode = recvData.substr(9, 3);
					int code = atoi(retcode.c_str());
					if(code < 200 || code >300)
					{
						cout << "http request failed:" << code << endl;
						close(_socketfd);
						return -1;
					}
					else
					{
						size_t length_pos = recvData.find("Content-Length:");
						if(length_pos == string::npos)
						{
							cout << "no content length" << endl;
							close(_socketfd);
							return -1;
						}
						else
						{
							size_t pos = recvData.find("\r", length_pos);
							string len = recvData.substr(length_pos+15, pos-length_pos-15);
							_content_length = atoi(len.c_str());
						}
					}
				}
			}
			recv_length += num;
			if(bGotHeader && recv_length >= header_length){
				string data = recvData.substr(header_length, recvData.size()-header_length);
				_buffer->Put(data.c_str(), data.length());
				_recv_length += data.length();
				break;
			}
			
		}
		else
			break;
	}

	cout << "content length = " << _content_length << endl;
	return 0;
}

int HttpClient::Open(const string url)
{
	int ret = ParseUrl(url);
	if(ret != 0)
		return ret;
	ret = Connect();
	if(ret != 0)
		return ret;

	_buffer = new RingBuffer();
	_buffer->Init();

	ret = ReadHeader();
	if(ret != 0)
		return ret;
	_recving = true;
	_readThread = std::thread(::ReadThread, (void*)this);

	return 0;
}

void HttpClient::Seek(unsigned long long pos)
{
	int offset = pos - _read_pos;
	while(_buffer->CurrentSize() < offset)
	{
		usleep(10000);
		continue;
	}
	_buffer->Skip(offset);
}

int HttpClient::Read(char* buffer, int size)
{
	while(_recving && _buffer->CurrentSize() < size)
	{
		usleep(10000);
		continue;
	}
	int ret = _buffer->Get(buffer, size);
	_read_pos += ret;
	return ret;
}