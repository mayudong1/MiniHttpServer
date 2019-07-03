#ifndef __HTTP_H__
#define __HTTP_H__

#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <atomic>
#include "buffer.h"

class HttpClient
{
public:
	HttpClient();
	~HttpClient();

	void Test();
	int Open(const std::string url);
	int Read(char* buffer, int size);
	void Seek(unsigned long long pos);

	void ReadThread();
private:
	unsigned long long _pos;
	std::string _url;
	std::string _hostName;
	std::string _hostIp;
	short _port;
	std::string _fileName;
	unsigned long long _content_length;
	unsigned long long _recv_length;
	unsigned long long _read_pos;
	std::atomic_bool _recving;


	int _socketfd;
	std::thread _readThread;
	RingBuffer* _buffer;

private:
	int ParseUrl(const std::string url);
	int Connect();
	int ReadHeader();

};


#endif