#include <iostream>  
#include "./include/cepollserver.h"  
using namespace std;  
  
int main()  
{       
        CEpollServer  robot_server;  //服务器对象       
        //AES my_aes; //加密工具对象       
        robot_server.InitServer("127.0.0.1",3557,"1234567890123456","1234567890123456");//初始化服务器参数          
        //开始监听
        robot_server.Run();  
  
        return 0;  
}  
