#include "utils.h"
#include <iostream>
#include <string>
#include <thread>
#include <list>
#include "clientManager.h"

using namespace std;

int main(int argc, char** argv)
{
	bool exit=false;
	//start/open the server in a free port : 1067 
	cout<<"Server opening port\n";
	int serverPortId=initServer(1067);
	cout<<"Server port opened, waiting for connections\n";
	while(!exit){
		//wait for connections
		while(!checkClient()) usleep(100);
			//attend client
		int clientId=getLastClientID();
		cout<<"Client "<<clientId<<" connected\n";
		 //send/recv messages
		thread* th=new thread(clientManager::resolveClientMessages,clientId);
	}
	close(serverPortId);
 return 0;
}


