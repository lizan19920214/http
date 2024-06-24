#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <map>
#include <vector>
#include <thread>
#include <sstream>
#include <string.h>

class httpd
{
public:
    bool InitServer();
    bool httpRsp(int client);
};