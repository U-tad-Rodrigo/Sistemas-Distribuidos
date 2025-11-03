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
		tipoMsg=unpack<personaFuncs>(buffer);
			//si tipo mensaje
		switch(tipoMsg){
			case construyePersonaParams: {

				string p1;
				int p2;
				//instanciar persona con parámetros
				//desempaquetar parametros de entrada: nombre(string), edad(int)
				p1.resize(unpack<int>(buffer));
				unpackv(buffer,(char*)p1.data(),(int)p1.size());
				p2=unpack<int>(buffer);
				Persona p(p1,p2);
				//almacenar la persona
				clientManager::instanciasPersonas[clientId]=p;
				//enviar ack/exito
					//limpiar buffer
					buffer.clear();
					pack(buffer,ackMSG);
					sendMSG(clientId,buffer);

			}break;

			case construyePersona: {

				//instanciar persona sin parámetros
				Persona p;
				//almacenar la persona
				clientManager::instanciasPersonas[clientId]=p;
				//enviar ack/exito
				//limpiar buffer
				buffer.clear();
				pack(buffer,ackMSG);
				sendMSG(clientId,buffer);

			}break;

			case destruyePersona: {

				//destructor persona sin parametros
				//instanciar persona sin parámetros
				//eliminar la persona
				clientManager::instanciasPersonas.erase(clientId);
				//enviar ack/exito
				//limpiar buffer
				buffer.clear();
				pack(buffer,ackMSG);
				sendMSG(clientId,buffer);

			}break;

			case setNombreF: {

				string p1;
				//instanciar persona con parámetros
				//desempaquetar parametros de entrada: nombre(string), edad(int)
				p1.resize(unpack<int>(buffer));
				unpackv(buffer,(char*)p1.data(),(int)p1.size());

				//invocar funcion
				clientManager::instanciasPersonas[clientId].setNombre(p1);
				//enviar ack/exito
				//limpiar buffer
				buffer.clear();
				pack(buffer,ackMSG);
				sendMSG(clientId,buffer);

			}break;

			case setEdadF: {

				int p1;
				//instanciar persona con parámetros
				//desempaquetar parametros de entrada: nombre(string), edad(int)
				p1 = unpack<int>(buffer);

				//invocar funcion
				clientManager::instanciasPersonas[clientId].setEdad(p1);
				//enviar ack/exito
				//limpiar buffer
				buffer.clear();
				pack(buffer,ackMSG);
				sendMSG(clientId,buffer);

			}break;

			case getNombreF: {

				//invocar funcion
				string resultado = clientManager::instanciasPersonas[clientId].getNombre();
				//enviar resultado
				//limpiar buffer
				buffer.clear();
				pack(buffer,resultado.size());
				packv(buffer,(char*)resultado.data(),(int)resultado.size());
				pack(buffer,ackMSG);
			}break;

			case getEdadF: {

				//invocar funcion
				int resultado = clientManager::instanciasPersonas[clientId].getEdad();
				//enviar resultado
				//limpiar buffer
				buffer.clear();
				pack(buffer,resultado);
				pack(buffer,ackMSG);
			}break;
			default:{
				std::cout<<"ERROR: tipo de mensaje no valido\n";
			}break;
		}
		sendMSG(clientId,buffer);
	//mientras no tipo closed
	}while(!salir);
	closeConnection(clientId);
}