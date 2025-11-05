#include "database.h"
#include "utils.h"
#include "clientManager.h"

database::database(string name){
}

bool database::addRecord(string key,string data){
	return false;
}

bool database::addRecordSet(string key,vector<string> dataSet){
	//crear buffer
	vector<unsigned char> buffer;
	int serverId=clientManager::connectionIds[this];
	
	//empaquetar tipo
	pack(buffer,addRecordSetF);
	pack(buffer,(int)key.size());
	packv(buffer,(char*)key.data(),(int)key.size());
	//empquetar tamaño vector
	pack(buffer,(int)dataSet.size());
	//empaquetar por cada del vector
	for( auto &dato: dataSet){
		pack(buffer,(int)dato.size());
		packv(buffer,(char*)dato.data(),(int)dato.size());
	}
	
	//enviar
	sendMSG(serverId,buffer);
	//recibir resultado
	buffer.clear();
	recvMSG(serverId,buffer);
	bool res=unpack<bool>(buffer);
	//desempaquetar ack
	if(unpack<msgTypes>(buffer)!=ack)
		cout<<"ERROR Respuesta no válida "<<__FILE__<<":"<<__LINE__<<endl;

	return res;
}
string database::getRecord(string key,int position)
{
	return "";
}

database::~database(){
	
}
