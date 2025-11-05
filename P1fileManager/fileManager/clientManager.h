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
	


	static inline map<FileManager*, int> connectionIds;
	static inline map<int,FileManager> instanciasFileManager; //mapa para servidor

	static void resolveClientMessages(int clientId);
	
};