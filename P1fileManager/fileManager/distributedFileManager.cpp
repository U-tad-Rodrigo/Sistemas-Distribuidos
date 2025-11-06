/*Rodrigo Fernández
* 05/11/2205
 */

#include "fileManager.h"
#include "utils.h"
#include "clientManager.h"

#define SERVERIP "127.0.0.1"
#define SERVERPORT 1067

// Mapa local para el cliente (no usar el del servidor)
static map<FileManager*, int> connectionIds;
/**
* @brief FileManager::FileManager Constructor without parameters of the FileManager class (empty). 
*
*/

FileManager::FileManager() {
	vector<unsigned char> buffer;
	connectionIds[this] = initClient(SERVERIP, SERVERPORT).serverId;
	int serverId = clientManager::connectionIds[this];

	pack(buffer, constructorFilemanager);
	sendMSG(serverId, buffer);

	recvMSG(serverId, buffer);
	if (buffer.size() > 0) {
		ready = true;
	}
}

/**
* @brief FileManager::FileManager Destructor without parameters of the FileManager class. 
*
*/
FileManager::~FileManager(){
	int serverID = clientManager::connectionIds[this];
	vector<unsigned char> buffer;
	pack(buffer, destructorFilemanager);
	sendMSG(serverID, buffer);
	recvMSG(serverID, buffer);
}
/**
* @brief FileManager::FileManager Constructor of the FileManager class. It receives by parameters the directory
* that this class will use to index, store and read files. It is recommended to use a full path to the directory,
* from the root of the file system.
*
* @param path Path to the directory you want to use.
*/
FileManager::FileManager(string path) {
	connectionIds[this] = initClient(SERVERIP, SERVERPORT).serverId;
	int serverId = clientManager::connectionIds[this];
	vector<unsigned char> buffer;

	pack(buffer, constructorFilemanagerParams);
	pack(buffer, (int)path.size());
	packv(buffer, (unsigned char *)path.data(), (int)path.size());

	sendMSG(serverId, buffer);

	recvMSG(serverId, buffer);

	if (buffer.size() > 0) {
		ready = true;
	}
}
/**
 * @brief FileManager::listFiles Used to access the list of files stored in the path
 * that was used in the class constructor. Only lists files, directories are ignored.
 * 
 */
vector<string> FileManager::listFiles(){
	vector<unsigned char> buffer;
	int serverId = clientManager::connectionIds[this];

	pack(buffer, listFilesF);
	sendMSG(serverId, buffer);
	recvMSG(serverId, buffer);

	vector<string> resultado;
	resultado.resize(unpack<int>(buffer));
	for (auto& fileName : resultado) {
		fileName.resize(unpack<int>(buffer));
		unpackv(buffer, (char*)fileName.data(), (int)fileName.size());
	}
	return resultado;

}
/**
 * @brief FileManager::readFile Given the name of a file stored in the directory used in the constructor,
 * the variable "data" will be filled with the contents of the file
 *
 * @param fileName Name of the file to read
 * @param data File data
 */
void FileManager::readFile(string fileName, vector<unsigned char> &data){
	int serverId = clientManager::connectionIds[this];
	vector<unsigned char> buffer;
	pack(buffer, readFileF);
	pack(buffer, (int)fileName.size());
	packv(buffer, (unsigned char *)fileName.data(), (int)fileName.size());

	sendMSG(serverId, buffer);
	recvMSG(serverId, buffer);

	data.resize(unpack<int>(buffer));
	unpackv(buffer, data.data(), (int)data.size());
}
/**
 * @brief FileManager::writeFile Given a new name of a file to be stored in the directory used in the constructor,
 * the contents of the data array stored in "data" will be written. It will overwrite a file in the directory if it has the same name.
 *
 * @param fileName Name of the file to write.
 * @param data Data of the file.
 */
void FileManager::writeFile(string fileName, vector<unsigned char> &data){
	int serverId = clientManager::connectionIds[this];
	vector<unsigned char> buffer;
	pack(buffer, writeFileF);
	pack(buffer, (int)fileName.size());
	packv(buffer, (unsigned char *)fileName.data(), (int)fileName.size());

	pack(buffer, (int)data.size());
	packv(buffer, data.data(), (int)data.size());
	sendMSG(serverId, buffer);
	recvMSG(serverId, buffer);
}