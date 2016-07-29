#ifndef __HTTPHEADER_H_
#define __HTTPHEADER_H_

#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <iostream>
#include <algorithm>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>

#include "AutoLock.h"

using namespace std;

typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

#endif