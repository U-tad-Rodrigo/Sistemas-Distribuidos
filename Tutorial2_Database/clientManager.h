#pragma once
#include "utils.h"
#include <string>
#include "database.h"

using namespace std;

class clientManager{
public:
	typedef enum{
		constructorDatabase,
		destructorDatabase,
		addRecordF,
		addRecordSetF,
		getRecordF,
		ack
	}msgTypes;


	static inline map<database*, int> connectionIds;
	static inline map<int, database> instanciasDatabase;
	
	static void resolveClientMessages(int clientId);
	
};