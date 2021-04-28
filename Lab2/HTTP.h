#ifndef HTTP_H
#define HTTP_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netdb.h> 
#include <vector>
#include "TCPServer.h"

using namespace std;
struct Message{
	string type;
	string html;
}



class HTTP
{
  private:
	
  public:
	setHTTP(char **argv);
	getCon();
};

#endif




























