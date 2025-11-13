/*Rodrigo Fernández
* 05/11/2205
 */
#pragma once

#include "utils.h"
#include <string>
#include "fileManager.h"

using namespace std;
typedef enum{
		constructorFilemanager,
		constructorFilemanagerParams,
		destructorFilemanager,
		listFilesF,
		readFileF,
		writeFileF,
		ack
}msgTypes;

class clientManager{
public:
	//USAR EN CLIENTE
	static inline map<FileManager*, int> connectionIds;
	//USAR EN SERVER
	static inline map<int,FileManager> instanciasFileManager;

	static void resolveClientMessages(int clientId);
	
};