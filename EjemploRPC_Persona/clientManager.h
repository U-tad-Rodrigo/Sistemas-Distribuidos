#pragma once
#include "utils.h"
#include "persona.h"
using namespace std;
typedef enum{
	ackMSG
}personaFuncs;

class clientManager{

		public:
		
			static inline map<Persona*,connection_t > connectionIds;
			
			static inline bool salir=false;			
			static void atiendeCliente(int clientId);
};