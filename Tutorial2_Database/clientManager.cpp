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
		
			case addRecordSetF:
			{
				string p1;
				vector<string> p2;
				//desempaquetar primer parámetro
				p1.resize(unpack<int>(buffer));
				unpackv(buffer,(char*)p1.data(),(int)p1.size());
				
				//segundo parámetro
				p2.resize(unpack<int>(buffer));
				for(auto &dato : p2)
				{
					dato.resize(unpack<int>(buffer));
					unpackv(buffer,(char*)dato.data(),(int)dato.size());
				}
				
				bool res=clientManager::instanciasDatabase[clientId].addRecordSet(p1,p2);
				
				buffer.clear();
				pack(buffer,res);
				pack(buffer,ack);
				sendMSG(clientId,buffer);
				
			}break;
			default:
			break;
		}	
		
				
	}while(!logOut);
	//close connection

	closeConnection(clientId);

}
