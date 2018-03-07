#include <iostream>  
#include "./include/cepollserver.h"  
  
using namespace std;  
  
int main()  
{  
        CEpollServer  theApp;  
        theApp.InitServer("127.0.0.1",3557);  
        theApp.Run();  
  
        return 0;  
}  
