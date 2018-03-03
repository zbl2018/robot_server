#include <sys/epoll.h>   
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <unistd.h>  
#include <fcntl.h>  
#include <netinet/in.h>  
//返回值的宏定义，在开发linux环境时，用0表示成功  
#define RES_SUC 0  
#define RES_FAIL -1  
  
#define FD_NULL 0  
#define null 0  
#define MAX_EPOLL_FD 1024  
#define LISTEN_SOCK_QUEUE 1024  
  
#define WORK_STATE_STOP 0  
#define WORK_STATE_RUN 1  
//服务器的监听端口  
#define LISTNE_PORT 3557  
  
#define INVALID_SOCKET -1  
  
#define MAX_EPOLL_EVENTS 1024  
//epoll在无事件发生时的最大返回间隔时间  
#define EPOLL_WAIT_INTERVAL 100   
  
#define  MAX_BUF_SIZE_READ 1024  
  
static int g_fd_epoll = FD_NULL;  
static int g_fd_listen_sock = FD_NULL;  
static int g_run_server = WORK_STATE_RUN;  

//注册一个socket到epoll  
int reg_socket(int new_sock)  
{  
    if(new_sock <= 0)  
        return RES_FAIL;  
    struct epoll_event new_events;  
    new_events.data.fd = new_sock;  
    new_events.events = EPOLLIN;//我们只关注有数据到来  
    if(epoll_ctl(g_fd_epoll, EPOLL_CTL_ADD,new_sock, &new_events) < 0){  
        printf("[reg_socket] register socket into epoll faill!\n");  
        return RES_FAIL;  
    }  
    else          
        return RES_SUC;               
} 
//处理就绪的业务socket  
int handle_business_socket(int ready_socket)  
{  
    if(ready_socket == INVALID_SOCKET)  
    {  
        printf("[handle_business_socket] socket:%d is invalid!\n", ready_socket);  
        return RES_FAIL;  
    }  
    char* read_buf = (char* )malloc(sizeof(char) * MAX_BUF_SIZE_READ);  
    memset(read_buf, '\0', MAX_BUF_SIZE_READ);  
    int read_len = read(ready_socket, read_buf, MAX_BUF_SIZE_READ);  
    if(read_len > 0)  
    {  
        printf("[handle_business_socket] recv msg from socket:%d, content:%s\n", ready_socket, read_buf);  
        if(write(ready_socket, read_buf, read_len) != read_len)  
        {  
            printf("[handle_business_socket] write msg into socket:%d fail! content:[%s]!\n", ready_socket, read_buf);  
        }  
    }  
    free(read_buf);  
} 
//处理就绪的监听socket,接收新的连接进来，并为之产生一个业务socket  
int handle_listen_socket()  
{  
    int new_sock = INVALID_SOCKET;  
    new_sock = accept(g_fd_listen_sock, NULL, 0);  
    if(new_sock == INVALID_SOCKET)  
    {  
        printf("[handle_listen_socket] accept new socket faill!\n");  
        return RES_FAIL;  
    }  
          
    //设置监听socket的状态为非阻塞  
    int opt = 1;  
    opt = fcntl(new_sock, F_GETFL, 0);  
    if(opt == -1 || fcntl(new_sock, F_SETFL, opt | O_NONBLOCK) == -1)  
    {  
        printf("[handle_listen_socket] set socket state: O_NONBLOCK fail!\n");  
        close(new_sock);  
        return RES_FAIL;  
    }  
      
    //注册监听socket到epoll  
    if(reg_socket(new_sock) == RES_FAIL)  
    {  
        printf("[handle_listen_socket] reg_socket:%d fail!\n", new_sock);  
        return RES_FAIL;  
    }  
    printf("[handle_listen_socket] accept new socket:%d suc!\n", new_sock);  
    return RES_SUC;  
} 
//主循环体  
int main_loop()  
{  
    //创建一个epoll事件的缓冲区，以便存放就绪的文件描述符  
    struct epoll_event* epoll_events = null;  
    epoll_events = (struct epoll_event*)malloc(sizeof(struct epoll_event) * MAX_EPOLL_EVENTS);  
    if(epoll_events == null)  
    {  
        printf("[main_loop] allocating memory for epoll events faill!\n");  
        return RES_FAIL;  
    }  
    int fd_ready_num = 0;  
    printf("[main_loop] server start success at port:%d!\n", LISTNE_PORT);  
    while(g_run_server == WORK_STATE_RUN)  
    {  
        //使用epoll来监控所有注册进来的socket，该步骤将以阻塞的方式用epoll_wait等待就绪的描述符，当一直没有描述符就绪时，最多将等待EPOLL_WAIT_INTERVAL毫秒之后，也会返回；  
        fd_ready_num = epoll_wait(g_fd_epoll, epoll_events, MAX_EPOLL_EVENTS, EPOLL_WAIT_INTERVAL);  
        //epoll_wait返回时说明有就绪的socket或者等待的超时时间到了  
        int i = 0;  
        for(i = 0; i < fd_ready_num; i++)  
        {  
            if(epoll_events[i].data.fd == g_fd_listen_sock)  
            {//说明当前的描述符是一个监听socket  
                if(handle_listen_socket() == RES_FAIL)  
                {  
                    printf("[main_loop] handle listen socket faill!\n");  
                }  
                continue;  
            }  
            //以下步骤将处理业务socket  
            if(handle_business_socket(epoll_events[i].data.fd) == RES_FAIL)  
            {  
                printf("[main_loop] handle business socket:%s faill!\n", epoll_events[i].data.fd);  
            }  
        }  
    }  
}  

/* 
*初始化监听socket，成功则返回创建的监听socket，否则返回RES_FAIL 
*/  
int init_listen_socket()  
{  
    printf("[init_listen_socket] will init listen socket!\n");  
    //创建监听socket的句柄  
    g_fd_listen_sock = socket(AF_INET, SOCK_STREAM, 0);  
    if(INVALID_SOCKET == g_fd_listen_sock)  
    {  
        printf("[init_listen_socket] create listen socket fail!\n");  
        return RES_FAIL;  
    }  
    //设置监听socket的状态为非阻塞  
    int opt = 1;  
    opt = fcntl(g_fd_listen_sock, F_GETFL, 0);  
    if(opt == -1 || fcntl(g_fd_listen_sock, F_SETFL, opt | O_NONBLOCK) == -1)  
    {  
        printf("[init_listen_socket] set socket state: O_NONBLOCK fail!\n");  
        close(g_fd_listen_sock);  
        return RES_FAIL;  
    }  
      
    //绑定socket到本地端口  
    struct sockaddr_in serv_addr;  
    serv_addr.sin_family = AF_INET;  
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // 使用本地通配地址  
    serv_addr.sin_port = htons(LISTNE_PORT);   
    if(bind(g_fd_listen_sock, (struct sockaddr *)&serv_addr, sizeof( serv_addr)) != RES_SUC)  
    {  
        printf("[init_listen_socket] bind socket to port %d fail!\n", LISTNE_PORT);  
        close(g_fd_listen_sock);  
        return RES_FAIL;  
    }  
    //开启socket的监听状态  
    if(listen(g_fd_listen_sock, LISTEN_SOCK_QUEUE) == -1)  
    {  
        printf("[init_listen_socket] listen socket fail!\n");  
        close(g_fd_listen_sock);  
        return RES_FAIL;  
    }  
      
    //注册监听socket到epoll  
    if(reg_socket(g_fd_listen_sock) == RES_FAIL)  
    {  
        printf("[init_listen_socket] reg_socket fail!\n");  
        return RES_FAIL;  
    }  
    printf("[init_listen_socket] init listen socket suc!\n");  
    return g_fd_listen_sock;  
}

/** 
*初始化服务端程序，成功返回RES_SUC 
*/  
int init()  
{  
    //创建epoll的句柄  
    g_fd_epoll = epoll_create(MAX_EPOLL_FD);  
    if(g_fd_epoll <= 0)  
    {  
        printf("[init] epoll_create fail!\n");  
        return RES_FAIL;  
    }  
    //初始化监听socket  
    if(init_listen_socket() == RES_FAIL)  
    {  
        printf("[init] epoll_create fail!\n");  
        return RES_FAIL;  
    }  
      
    return RES_SUC;  
}  

int main()  
{  
    printf("[main] will start test server!\n");  
    //初始化测试服务器  
    if(init() == RES_FAIL)  
    {  
        printf("[main] init fail!\n");  
        return RES_FAIL;  
    }  
    // 开始长循环工作  
    if(main_loop() == RES_FAIL)  
    {  
        printf("[main] main_loop fail!\n");  
        return RES_FAIL;  
    }  
    printf("[main] test server will exit!\n");  
    return RES_SUC;  
}  
