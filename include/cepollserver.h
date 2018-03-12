#ifndef  C_EPOLL_SERVER_H  
#define  C_EPOLL_SERVER_H  
  
#include <sys/epoll.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <fcntl.h>  
#include <arpa/inet.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <iostream>  
#include <pthread.h>  
#include <unistd.h>
#include <cstring>
#include"json/json.h"
#include"AES.h"
#include"MyDB.h"   
#define _MAX_SOCKFD_COUNT 65535  
  
class CEpollServer  
{  
        public:  
                CEpollServer();  
                ~CEpollServer();  
  
                bool InitServer(const char* chIp, int iPort,char temp_key[],char temp_iv[]);
                 int Deal_EpollIn_SomeThing(int socket_fd,char buffer[]);
                int Deal_EpollOUT_Encryption(char buffer[]);
                    
                //void Listen();
                // 监听线程，处理新的socket连接,加入epoll事务  
                static void ListenThread( void* lpVoid );
                //监听epoll事务，调用Deal_EpollIn_SomeThing()处理接收的消息             
                void Run();  
  
        private:
                AES my_aes;
		MyDB my_db;  
                int m_iEpollFd;  
                int m_isock;  
                pthread_t m_ListenThreadId;// 监听线程句柄  
  
};  
  
#endif 
