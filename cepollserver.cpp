#include "./include/cepollserver.h"  

using namespace std;  

CEpollServer::CEpollServer()  
{ 
     
}  

CEpollServer::~CEpollServer()  
{  
    close(m_isock);  
}  

bool CEpollServer::InitServer(const char* pIp, int iPort,char temp_key[],char temp_iv[])  
{  
    //初始化加密秘钥，向量
    if(false==my_aes.initKV(temp_key,temp_iv))
    {
        printf("fail to initKV!\n");  
        return false;  
    }
    //初始化数据库链接
    if(false==my_db.initDB("localhost", "root", "4817", "server"/*数据库名称*/))
    {
        printf("fail to init database!\n");
        return false;
    }

    //创建epoll句柄
    m_iEpollFd = epoll_create(_MAX_SOCKFD_COUNT);  

    //设置非阻塞模式  
    int opts = O_NONBLOCK;  
    if(fcntl(m_iEpollFd,F_SETFL,opts)<0)  
    {  
        printf("fail to set the mode of ’O_NONBLOCK‘!\n");  
        return false;  
    }  

    m_isock = socket(AF_INET,SOCK_STREAM,0);  
    if ( 0 > m_isock )  
    {  
        printf("socket error!\n");  
        return false;  
    }
    sockaddr_in listen_addr;  
    listen_addr.sin_family=AF_INET;  
    listen_addr.sin_port=htons ( iPort );  
    listen_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    listen_addr.sin_addr.s_addr=inet_addr(pIp);
    int ireuseadd_on = 1;//支持端口复用  
    setsockopt(m_isock, SOL_SOCKET, SO_REUSEADDR, &ireuseadd_on, sizeof(ireuseadd_on) );  
    if(bind ( m_isock, ( sockaddr * ) &listen_addr,sizeof ( listen_addr ) ) !=0 )  
    {  
        perror("bind error:");
        printf("bind error\n");  
        return false;  
    } 
    if ( listen (m_isock, 20) <0 )
    {  
        printf("listen error!\n");  
        return false;  
    }  
    else  
    {  
    printf("starting listen...\n");  
    }  
    // 监听线程，此线程负责接收客户端连接，加入到epoll中,this 指针传入ListenThread所在类对象指针
    if (pthread_create( &m_ListenThreadId, 0, ( void * ( * ) ( void * ) ) ListenThread, this ) != 0 )  
    {  
        printf("create listen thread failly!");  
        return false;  
    }  
} 
int CEpollServer::Deal_EpollIn_SomeThing(int socket_fd,char buffer[]){
    Json::Reader reader;
    Json::Value value;
    //decryption
    string cipher=my_aes.decrypt(buffer);
    printf("解密：%s\n",cipher);
    if (reader.parse(cipher, value))
    {
        std::string out = value["name"].asString();
        std::cout << out << std::endl;
        cout<<value["key"].asInt()<<endl;
        // const Json::Value arrayObj = value["array"];
        // for (int i=0; i<arrayObj.size(); i++)
        // {
        //     out = arrayObj[i]["key2"].asString();
        //     std::cout << out;
        //     if (i != arrayObj.size() - 1)
        //     std::cout << std::endl;
        // }
    }
    // if()
    // send(client_socket, sendbuff, strlen(sendbuff)+1, MSG_NOSIGNAL)
    cipher="this is a test!";
    cipher=my_aes.encrypt(cipher);
    if(send(socket_fd,cipher.c_str(),cipher.length()+1,MSG_NOSIGNAL)<=0){
        printf("fail to send message to client!\n");
    }
    else{
        printf("send successfully!\n");
    }
} 
int Deal_EpollOUT_Encryption(char buffer[]){

}
// 监听线程  
void CEpollServer::ListenThread( void* lpVoid )  
{  
    CEpollServer *pTerminalServer = (CEpollServer*)lpVoid;  
    sockaddr_in remote_addr;  
    int len = sizeof (remote_addr);  
    while ( true )  
    {   
        //从处于 established 状态的连接队列头部取出一个已经完成的连接，
        //如果这个队列没有已经完成的连接，accept()函数就会阻塞，直到取出队列中已完成的用户连接为止.
        int client_socket = accept (pTerminalServer->m_isock, ( sockaddr * ) &remote_addr,(socklen_t*)&len );  
        if ( client_socket < 0 )  
        {  
            printf("Server Accept failly!, client_socket: %d\n", client_socket);  
            continue;  
        }  
        else  
        {  
            struct epoll_event ev;  
            ev.events = EPOLLIN | EPOLLERR | EPOLLHUP;  
            ev.data.fd = client_socket;     //记录socket句柄  
            epoll_ctl(pTerminalServer->m_iEpollFd, EPOLL_CTL_ADD, client_socket, &ev);  
        }  
    }
} 
void CEpollServer::Run()  
{  
    while ( true )  
    {  
    struct epoll_event events[_MAX_SOCKFD_COUNT];  
    int num_fds = epoll_wait( m_iEpollFd, events,  _MAX_SOCKFD_COUNT, -1 );  
    for (int i = 0; i < num_fds; i++)  
    {  
        int client_socket = events[i].data.fd;  
        char buffer[1024];//每次收发的字节数小于1024字节  
        memset(buffer, 0, 1024);
        //当前socket_fd触发EPOLLIN事件
        if (events[i].events & EPOLLIN)//监听到读事件，接收数据  
        {  
            int rev_size = recv(events[i].data.fd,buffer,1024,0);  
            if( rev_size <= 0 )  
            {  
                //客户段断开，发送数据长度为0
                cout << "recv error: recv size: " << rev_size << endl;  
                struct epoll_event event_del;  
                event_del.data.fd = events[i].data.fd;  
                event_del.events = 0;  
                epoll_ctl(m_iEpollFd, EPOLL_CTL_DEL, event_del.data.fd, &event_del);  
            }  
            else  
            {  
                //处理接收到的数据
                Deal_EpollIn_SomeThing(client_socket,buffer);
                printf("Received Msg Content of client %d:%s\n",events[i].data.fd,buffer);  
                struct epoll_event ev;  
                ev.events = EPOLLOUT | EPOLLERR | EPOLLHUP | EPOLLIN;  
                ev.data.fd = client_socket;     //记录socket句柄  
                epoll_ctl(m_iEpollFd, EPOLL_CTL_MOD, client_socket, &ev);  
            }  
        }  
        //当前socket_fd未触发EPOLLIN事件，触发了EPOLLOUT事件，可以向客户端发送数据
        else if(events[i].events & EPOLLOUT)//监听到写事件，发送数据  
        {  
            // char sendbuff[1024];  
            // sprintf(sendbuff, "Hello, client fd: %d\n", client_socket);  
            // int sendsize = send(client_socket, sendbuff, strlen(sendbuff)+1, MSG_NOSIGNAL);  
            // if(sendsize <= 0)  
            // {  
            //     struct epoll_event event_del;  
            //     event_del.data.fd = events[i].data.fd;  
            //     event_del.events = 0;  
            //     epoll_ctl(m_iEpollFd, EPOLL_CTL_DEL, event_del.data.fd, &event_del);  
            // }  
            // else  
            // {  
            //     printf("Server reply msg ok! buffer: %s\n", sendbuff);  
            //     struct epoll_event ev;  
            //     ev.events = EPOLLIN | EPOLLERR | EPOLLHUP;  
            //     ev.data.fd = client_socket;     //记录socket句柄  
            //     epoll_ctl(m_iEpollFd, EPOLL_CTL_MOD/*修改*/, client_socket, &ev);  
            // }  
        }  
        else  
        {  
            cout << "EPOLL ERROR\n" <<endl;  
            epoll_ctl(m_iEpollFd, EPOLL_CTL_DEL, events[i].data.fd, &events[i]);  
        }  
    }  
    }  
}