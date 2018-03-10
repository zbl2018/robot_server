/*socket tcp客户端*/
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include"./include/json/json.h"
#include"./include/AES.h"

#define SERVER_PORT 3557

int main()
{
	// AES my_aes;
	// my_aes.initKV("1234567890123456","1234567890123456");  

	AES my_aes;
    string text = "hello zbl !";  
    cout<<"text : "<<text<<endl;  
    my_aes.initKV("1234567890123456","1234567890123456");  
    string cipherHex =my_aes.encrypt(text);  
    cout<<"cipher : "<<cipherHex<<endl;  
    //writeCipher(cipherHex); 
    my_aes.initKV("1234567890123456","1234567890123456");
    cipherHex =my_aes.decrypt(cipherHex);
    cout<<"cipher : "<<cipherHex<<endl; 






	int clientSocket;
	struct sockaddr_in serverAddr;
	char sendbuf[200];
	char recvbuf[1024];
	int iDataNum;
	if((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		return 1;
	}

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	if(connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
	{
		perror("connect");
		return 1;
	}

	printf("connect with destination host...\n");

	// std::string strValue = "{\"key1\":\"value1\",\"array\":[{\"key2\":\"value2\"},{\"key2\":\"value3\"},{\"key2\":\"value4\"}]}";
	Json::Value root;
	root["key"]=1111;
	root["name"]="zbl";
	string strValue= root.toStyledString();
	strcpy(sendbuf,my_aes.encrypt(strValue).c_str());
	printf("value:%s\n",strValue);
	while(1)
	{
		printf("Input your world:>");
		//scanf("%s", sendbuf);
		printf("\n");
		send(clientSocket, sendbuf, strlen(sendbuf), 0);
		if(strcmp(sendbuf, "quit") == 0)
			break;
		iDataNum = recv(clientSocket, recvbuf, 1024, 0);
		//recvbuf[iDataNum] = '\0';
		cipherHex =my_aes.decrypt(recvbuf);
    	cout<<"cipher : "<<cipherHex<<endl; 

		// recvbuf[iDataNum] = '\0';
		// string temp="this is a test!";
		// temp=my_aes.decrypt(my_aes.encrypt(temp));
		// printf("recv data of my world is: %s\n", temp);
		//memset(recvbuf,'0',200);
	}
	close(clientSocket);
	return 0;
}
