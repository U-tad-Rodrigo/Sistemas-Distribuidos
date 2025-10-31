#include "clientManager.h"


void clientManager::atiendeCliente(int clientId){
	personaFuncs tipoMsg;
	std::vector<unsigned char> buffer;
	//repetir
	do{
		//recibir mensaje
		recvMSG(clientId, buffer);
		//desempaquetar tipo de mensaje
		tipoMsg=unpack<personaFuncs>(buffer);
			//si tipo mensaje
		switch(tipoMsg){			default:{
				std::cout<<"ERROR: tipo de mensaje no valido\n";
			}break;
		}
		sendMSG(clientId,buffer);
	//mientras no tipo closed
	}while(tipoMsg!=PersonaDF);
	closeConnection(clientId);
}