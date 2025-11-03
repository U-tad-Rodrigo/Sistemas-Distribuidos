#pragma once
#include "utils.h"
#include "persona.h"
using namespace std;
typedef enum{ // Tipos de mensajes (un tipo por funcion)
    construyePersona,
    construyePersonaParams,
    destruyePersona,
    ackMSG
}personaFuncs;

class clientManager{

		public:
		
			static inline map<Persona*,int > connectionIds; // Para el cliente
			
			static inline bool salir=false;			
			static void atiendeCliente(int clientId);
};