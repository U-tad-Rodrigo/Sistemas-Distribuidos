#include "clientManager.h"
#include "database.h"
#include "utils.h"

using namespace std;

database::database(string name){
}

bool database::addRecord(string key,string data){
	return false;
}

bool database::addRecordSet(string key,vector<string> dataSet){
	vector<unsigned char> buffer;
	int serverId= clientManager::connectionIds[this];
	pack(buffer, clientManager::addRecordSetF);
	pack(buffer, (int) key.size());
	packv(buffer, (char *) key.data(), (int) key.size());
	pack(buffer, (int) dataSet.size());
	for (auto &dato: dataSet) {
		pack(buffer, (int) dato.size());
		packv(buffer, (char *) dato.data(), (int) dato.size());
	}

	sendMSG(serverId, buffer);
	buffer.clear();
	recvMSG(serverId, buffer);
	bool res = unpack<bool>(buffer);
	if(unpack<bool>(buffer)!= ackMSG) {
		cout<<"ERROR Respuesta no válida" << __FILE__ << ":" << __LINE__ << endl;
		return false;
	}

	return false;
}
string database::getRecord(string key,int position)
{
	return "";
}

database::~database(){
	
}
