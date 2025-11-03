
#include "utils.h"
#include <string>

using namespace std;

class clientManager{
public:
	typedef enum{
		ack
	}msgTypes;


	static inline map<string, int> connectionIds;
	
	static void resolveClientMessages(int clientId);
	
};