/*Rodrigo Fernández
 * 05/11/2205
 */

#include "clientManager.h"

void clientManager::resolveClientMessages(int clientId) {
	string userName="";
	vector<unsigned char> buffer;
	bool logOut=false;
	do {
		//receive a packet from client
		recvMSG(clientId,buffer);
		msgTypes type=unpack<msgTypes>(buffer);
		//switch type of the packet
		switch(type) {
			case constructorFilemanager: {
				FileManager fm;
				instanciasFileManager[clientId]=fm;
				//asociar id conexion
				connectionIds[&instanciasFileManager[clientId]]=clientId;

				//enviar ack
				buffer.clear();
				pack(buffer,ack);
				sendMSG(clientId,buffer);
			}break;
			case constructorFilemanagerParams: {
				FileManager fm;
				string path;
				//desempaquetar path
				path.resize(unpack<int>(buffer));
				unpackv(buffer,(char*)path.data(),(int)path.size());
				fm=FileManager(path);
				instanciasFileManager[clientId]=fm;
				//asociar id conexion
				connectionIds[&instanciasFileManager[clientId]]=clientId;

				buffer.clear();
				pack(buffer,ack);
				sendMSG(clientId,buffer);
			}break;
			case destructorFilemanager:
			{
				//eliminar instancia
				instanciasFileManager.erase(clientId);
				//enviar ack
				buffer.clear();
				pack(buffer,ack);
				sendMSG(clientId,buffer);
				logOut=true;
			}break;
			case listFilesF:
			{
				//invocar listFiles en servidor
				vector<string> resultado=instanciasFileManager[clientId].listFiles();
				//empaquetar resultado
				buffer.clear();
				pack(buffer,(int)resultado.size());
				for(auto &fileName: resultado){
					pack(buffer,(int)fileName.size());
					packv(buffer,(char*)fileName.data(),(int)fileName.size());
				}
				//enviar resultado
				sendMSG(clientId,buffer);
			}break;
			case readFileF: {
				string file;
				vector<unsigned char> data;
				//desempaquetar file
				file.resize(unpack<int>(buffer));
				unpackv(buffer,(char*)file.data(),(int)file.size());

				instanciasFileManager[clientId].readFile(file, data);
				//enviar resultado
				buffer.clear();
				pack(buffer,(int)data.size());
				packv(buffer,(char*)data.data(),(int)data.size());
				sendMSG(clientId,buffer);
			}break;
			case writeFileF: {
				string file;
				file.resize(unpack<int>(buffer));
				unpackv(buffer,(char*)file.data(),(int)file.size());

				vector<unsigned char> data;
				data.resize(unpack<int>(buffer));
				unpackv(buffer, (char *)data.data(), (int)data.size());
				instanciasFileManager[clientId].writeFile(file, data);

				buffer.clear();
				pack(buffer,ack);
				sendMSG(clientId,buffer);
			}break;
			default:
				break;
		}
	} while(!logOut);
	//close connection
	closeConnection(clientId);
}