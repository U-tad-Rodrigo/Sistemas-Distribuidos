#include "clientManager.h"

void clientManager::resolveClientMessages(int clientId){

	string userName="";
	vector<unsigned char> buffer;
	bool logOut=false;
	do{
	//receive a packet from client
		recvMSG(clientId,buffer);
		msgTypes type=unpack<msgTypes>(buffer);
	//switch type of the packet
		switch(type){
		//type login
			default:
			break;
		}
		buffer.clear();
		pack(buffer,ack);
		sendMSG(clientId,buffer);
		
	}while(!logOut);
	//close connection

	closeConnection(clientId);

}
