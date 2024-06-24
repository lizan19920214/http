/**
 * g++ -o httpd httpd.cpp -lpthread
 * 客户端使用curl命令测试:
 * 格式： curl -H 自定义请求头 -d 请求体 -X POST 接口地址
 * curl  -X POST -H "cmdid:1102" -d 'name=aa,sex=2,age=10'  http://127.0.0.1:8888
 * 
 * http协议格式
 * 响应行:请求方法 空格 请求URL 空格 请求版本 \r\n
 * 头部字段集合: 头部字段名:头部字段值 \r\n 
 *              头部字段名:头部字段值 \r\n
 *              头部字段名:头部字段值 \r\n...
 * 空行:\r\n
 * 消息正文
 * 
*/
#include "httpd.h"

//分割字符串
//返回分割后的字符串数组
std::vector<std::string> splitString(std::string str, std::string pattern)
{
    std::string::size_type pos;
    std::vector<std::string> result;
    str += pattern;
    int size = str.size();
    for (int i = 0; i < size; i++)
    {
        pos = str.find(pattern, i);
        if (pos < size)
        {
            std::string s = str.substr(i, pos - i);
            result.push_back(s);
            i = pos + pattern.size() - 1;
        }
    }

    return result;
}

void THreadFunc(void* arg, int conn)
{
    httpd* pHttpd = (httpd*)arg;
    
    //接收http请求
    char bodyBuff[1024] = {0};
    int recvSize = recv(conn, bodyBuff, 1024, 0);
    std::cout << bodyBuff << std::endl;

    std::string strMethod;
    std::string strUrl;
    std::string strVersion;
    std::map<std::string, std::string> mapHead;

    //第一次按照"\r\n\r\n"分割（0响应行+头部集合 1消息正文）
    std::vector<std::string> firstStr = splitString(bodyBuff, "\r\n\r\n");

    std::string strBody = firstStr[1];
    //第二次分割"\r\n" 0响应行 1-N头部集合
    std::vector<std::string> secondStr = splitString(firstStr[0], "\r\n");

    //响应行按照空格分割
    std::vector<std::string> thirdStr = splitString(secondStr[0], " ");
    strMethod = thirdStr[0];
    strUrl = thirdStr[1];
    strVersion = thirdStr[2];
   
    //从第二个数据开始，处理头部集合
    for (int i = 1; i < secondStr.size(); i++)
    {
        //第四次分割 ":" 处理头部集合
        std::vector<std::string> fourStr = splitString(secondStr[i], ":");
        mapHead.insert(std::make_pair(fourStr[0], fourStr[1]));
    }
    
    std::cout << "========================" << std::endl;
    std::cout << "method:" << strMethod << std::endl;
    std::cout << "url:" << strUrl << std::endl;
    std::cout << "version:" << strVersion << std::endl;
    for(auto it : mapHead)
    {
        std::cout << "key:" << it.first << " value:" << it.second << std::endl;
    }

    std::cout << "body:" << strBody << std::endl;
    std::cout << "========================" << std::endl;


    pHttpd->httpRsp(conn);
    
    //关闭套接字
    close(conn);
}

bool httpd::InitServer()
{
    int listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd < 0)
    {
        std::cout << "socket error" << std::endl;
        return false;
    }
    // 设置端口复用
    int opt = 1;
    setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    //定义服务器端口
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8888);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    //绑定端口
    if (bind(listenFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
    {
        std::cout << "bind error" << std::endl;
        return false;
    }

    //监听端口
    if (listen(listenFd, 5) < 0)
    {
        std::cout << "listen error" << std::endl;
        return false;
    }

    //客户端套接字
    char buff[1024] = {0};
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    int clientFd = 0;

    while(1)
    {
        int clientFd = accept(listenFd, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientFd < 0)
        {
            std::cout << "accept error" << std::endl;
            return false;
        }

        std::thread t(THreadFunc, this, clientFd);
        t.join();
    }

    close(listenFd);

    return true;
}

bool httpd::httpRsp(int client)
{
    char recvBuf[1024] = {0};
    sprintf(recvBuf, "HTTP/1.0 200 OK\r\n");
    send(client, recvBuf, strlen(recvBuf), 0);

    sprintf(recvBuf, "\r\n");
    send(client, recvBuf, strlen(recvBuf), 0);

    sprintf(recvBuf, "<HTML>\r\n");
    send(client, recvBuf, strlen(recvBuf), 0);

    sprintf(recvBuf, "<TITLE>\r\n");
    send(client, recvBuf, strlen(recvBuf), 0);

    sprintf(recvBuf, "HTTP Server\r\n");
    send(client, recvBuf, strlen(recvBuf), 0);

    sprintf(recvBuf, "</TITLE>\r\n");
    send(client, recvBuf, strlen(recvBuf), 0);

    sprintf(recvBuf, "<BODY>\r\n");
    send(client, recvBuf, strlen(recvBuf), 0);

    sprintf(recvBuf, "<div id=\"container\" style=\"width:500px\">\r\n");
    send(client, recvBuf, strlen(recvBuf), 0);
    
    sprintf(recvBuf, "<div id=\"header\" style=\"background-color:#FFA500;\">\r\n");
    send(client, recvBuf, strlen(recvBuf), 0);

    sprintf(recvBuf, "<h1 style=\"margin-bottom:0;\">www.baidu.com</h1></div>\r\n");
    send(client, recvBuf, strlen(recvBuf), 0);

    sprintf(recvBuf, "<div id=\"menu\" style=\"background-color:#FFD700;height:200px;width:100px;float:left;\">\r\n");
    send(client, recvBuf, strlen(recvBuf), 0);

    sprintf(recvBuf, "<b>MENU</b><br>\r\n");
    send(client, recvBuf, strlen(recvBuf), 0);

    sprintf(recvBuf, "HTML<br>\r\n");
    send(client, recvBuf, strlen(recvBuf), 0);

    sprintf(recvBuf, "CSS<br>\r\n");
    send(client, recvBuf, strlen(recvBuf), 0);

    sprintf(recvBuf, "JavaScript</div>\r\n");
    send(client, recvBuf, strlen(recvBuf), 0);

    sprintf(recvBuf, "<div id=\"content\" style=\"background-color:#EEEEEE;height:200px;width:400px;float:left;\">BODY</div>\r\n");
    send(client, recvBuf, strlen(recvBuf), 0);

    sprintf(recvBuf, "<div id=\"footer\" style=\"background-color:#FFA500;clear:both;text-align:center;\">runoob.com</div>\r\n");
    send(client, recvBuf, strlen(recvBuf), 0);


    sprintf(recvBuf, "</div>\r\n");
    send(client, recvBuf, strlen(recvBuf), 0);

    sprintf(recvBuf, "</BODY>\r\n");
    send(client, recvBuf, strlen(recvBuf), 0);

    sprintf(recvBuf, "</HTML>\r\n");
    send(client, recvBuf, strlen(recvBuf), 0);

    return true;
}

int main()
{
    httpd httpd;
    httpd.InitServer();
    return 0;
}