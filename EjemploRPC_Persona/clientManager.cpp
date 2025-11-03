#include "clientManager.h"


void clientManager::atiendeCliente(int clientId){
	personaFuncs tipoMsg;
	std::vector<unsigned char> buffer;
    bool salir = false;
	//repetir
	do{
		//recibir mensaje
		recvMSG(clientId, buffer);
		//desempaquetar tipo de mensaje
		tipoMsg = unpack<personaFuncs>(buffer);
			//si tipo mensaje
		switch(tipoMsg){
		    case construyePersona: {
		        // Instanciar persona sin parametros

		        // Almacenar la persona
		        // Enviar ACK
		    }
		    case construyePersonaParams: {
                string param1;
		        int param2;
		        // Instanciar persona con parametros
		            // Desempaquetar el tamano del mensaje
		            param1.resize(unpack<int>(buffer));
		            unpackv(buffer, (char*) param1.data(), (int)param1.size());
		            // Desempaquetar la edad
		            param2 = unpack<int>(buffer);

		        Persona p(param1, param2);

		        // Almacenar la persona
		        clientManager::instanciasPersonas[serverId] = p;

		        // Enviar ACK
		        buffer.clear();
		        pack(buffer, ackMSG);
		        sendMSG(clientId, buffer);
		    }
		    case destruyePersona: {

		    }



		    default:{
				std::cout<<"ERROR: tipo de mensaje no valido\n";
			}break;
		}
		sendMSG(clientId,buffer);
	//mientras no tipo closed
	}while(!salir);
	closeConnection(clientId);
}