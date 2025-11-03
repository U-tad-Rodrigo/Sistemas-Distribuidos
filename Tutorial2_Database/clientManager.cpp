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
			case addRecordSetF: {
				//haz lo que tenga que hacer
				string p1;
				vector<string> p2;
				p1.resize(unpack<int>(buffer));
				unpackv(buffer, (char*)p1.data(), (int)p1.size());
				p2.resize(unpack<int>(buffer));
				for (auto &dato: p2) {
					dato.resize(unpack<int>(buffer));
					unpackv(buffer, (char*)dato.data(), (int)dato.size());
				}
				bool res= instanciasDatabase[clientId].addRecordSet(p1,p2);
				buffer.clear();
				pack(buffer,res);
			}
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
