/*Rodrigo Fernández
* 05/11/2205
 */

#include "fileManager.h"
#include "utils.h"
#include "clientManager.h"

/**
* @brief FileManager::FileManager Constructor without parameters of the FileManager class (empty). 
*
*/

FileManager::FileManager()
{
	//connect to server
	int serverId=clientManager::connectionIds[this];
	//empaquetar tipo
	vector<unsigned char> buffer;
	pack(buffer,constructorFilemanager);
	//enviar
	sendMSG(serverId,buffer);
	//recibir ack
	buffer.clear();
	recvMSG(serverId,buffer);
	//comprobar ack
	if(unpack<msgTypes>(buffer)!=ack)
		cout<<"ERROR Respuesta no válida "<<__FILE__<<":"<<__LINE__<<endl;

	
}

/**
* @brief FileManager::FileManager Destructor without parameters of the FileManager class. 
*
*/
FileManager::~FileManager(){
	
}
/**
* @brief FileManager::FileManager Constructor of the FileManager class. It receives by parameters the directory
* that this class will use to index, store and read files. It is recommended to use a full path to the directory,
* from the root of the file system.
*
* @param path Path to the directory you want to use.
*/
FileManager::FileManager(string path){}
/**
 * @brief FileManager::listFiles Used to access the list of files stored in the path
 * that was used in the class constructor. Only lists files, directories are ignored.
 * 
 */
vector<string> FileManager::listFiles(){
	
	vector<string> resultado;
		//invocar listFiles en servidor
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
	
	
	
	
}
/**
 * @brief FileManager::writeFile Given a new name of a file to be stored in the directory used in the constructor,
 * the contents of the data array stored in "data" will be written. It will overwrite a file in the directory if it has the same name.
 *
 * @param fileName Name of the file to write.
 * @param data Data of the file.
 */
void FileManager::writeFile(string fileName, vector<unsigned char> &data){
	
}