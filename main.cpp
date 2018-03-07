#include <iostream>  
#include "./include/cepollserver.h" 
  
using namespace std;  
  
int main()  
{  
        CEpollServer  robot_server;
        //初始化服务器参数  
        robot_server.InitServer("127.0.0.1",3557);  
        //开始监听
        robot_server.Run();  
        
  
        return 0;  
}  
