
#include "utils.h"
#include <string>
#include "database.h"

using namespace std;
typedef enum{
		constructorDatabase,
		destructorDatabase,
		addRecordF,
		addRecordSetF,
		getRecordF,
		ack
}msgTypes;

class clientManager{
public:
	


	static inline map<database*, int> connectionIds;
	static inline map<int,database> instanciasDatabase; //mapa para servidor
	
	
	static void resolveClientMessages(int clientId);
	
};