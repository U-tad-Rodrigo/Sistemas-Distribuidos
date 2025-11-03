#pragma once
#include "utils.h"
#include "persona.h"
using namespace std;
typedef enum{
	construyePersona,
	construyePersonaParams,
	destruyePersona,
	ackMSG,
	setNombreF,
	getNombreF,
	setEdadF,
	getEdadF,
}personaFuncs;

class clientManager{

public:

	static inline map<Persona*,int > connectionIds;

	//cliente
	static inline map<int, Persona> instanciasPersonas;

	//mapa para servidor
	static inline bool salir=false;


	static void atiendeCliente(int clientId);
};