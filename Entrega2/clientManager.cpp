/*Rodrigo Fernández
 * 05/11/2025
 */

#include "clientManager.h"
#include "utils.h"

string clientManager::desempaquetaTexto(vector<unsigned char> &buffer){
	string mensaje;
	mensaje.resize(unpack<long int>(buffer));
	unpackv(buffer,(char*)mensaje.data(),mensaje.size());
	return mensaje;
}

void clientManager::atiendeCliente(int clientId)
{
	vector<unsigned char> bufferIn;
	bool salir=false;

	cerrojoClientes.lock();
	clientesConectados[clientId]=clientId;
	cerrojoClientes.unlock();

	while(!salir){
		//recibir paquete
		recvMSG(clientId,bufferIn);
		//desempaquetar tipo
		msgTypes type=unpack<msgTypes>(bufferIn);

		switch(type){
			case texto:{
				//desempaquetar mensaje
				string msg=desempaquetaTexto(bufferIn);
				//mostrar en servidor
				cout<<"cliente "<<clientId<<": "<<msg<<endl;
				//reenviar a otros
				reenviaTexto(clientId,msg);
			}break;

			case usuarios:{
				//enviar lista usuarios
				enviaListaUsuarios(clientId);
			}break;

			case privado:{
				//desempaquetar destinatario
				int idDestinatario=unpack<int>(bufferIn);
				//desempaquetar mensaje
				string msg=desempaquetaTexto(bufferIn);
				//enviar privado
				enviaMensajePrivado(clientId,idDestinatario,msg);
			}break;

			case exit:{
				//cerrar conexión
				cout<<"[SERVIDOR] Cliente "<<clientId<<" desconectado"<<endl;
				salir=true;
			}break;

			default:{
				ERRLOG("tipo mensaje no válido");
				salir=true;
			}break;
		}

		//limpiar buffer
		bufferIn.clear();
		//enviar ack
		pack(bufferIn,ack);
		sendMSG(clientId,bufferIn);
	}

	cerrojoClientes.lock();
	clientesConectados.erase(clientId);
	cerrojoClientes.unlock();

	closeConnection(clientId);
}

void clientManager::reenviaTexto(int idEmisor, string msg)
{
	vector<unsigned char> bufferOut;
	//empaquetar tipo
	pack(bufferOut,texto);
	//empaquetar id emisor
	pack(bufferOut,idEmisor);
	//empaquetar mensaje
	pack(bufferOut,msg.size());
	packv(bufferOut,msg.data(),msg.size());

	//reenviar a todos menos al emisor
	cerrojoClientes.lock();
	for(auto client : clientesConectados){
		if(client.first!=idEmisor)
			sendMSG(client.second,bufferOut);
	}
	cerrojoClientes.unlock();
}

string clientManager::obtenerListaIds()
{
	string lista="conectados: ";
	cerrojoClientes.lock();
	if(clientesConectados.empty()){
		lista="conectados: ninguno";
	}else{
		bool primero=true;
		for(auto client : clientesConectados){
			if(!primero) lista+=",";
			lista+=to_string(client.first);
			primero=false;
		}
	}
	cerrojoClientes.unlock();
	return lista;
}

void clientManager::enviaListaUsuarios(int clientId)
{
	string lista=obtenerListaIds();
	vector<unsigned char> bufferOut;
	//empaquetar tipo
	pack(bufferOut,usuarios);
	//empaquetar lista
	pack(bufferOut,lista.size());
	packv(bufferOut,lista.data(),lista.size());
	//enviar
	sendMSG(clientId,bufferOut);
}

void clientManager::enviaMensajePrivado(int idEmisor, int idDestinatario, string msg)
{
	vector<unsigned char> bufferOut;
	//empaquetar tipo
	pack(bufferOut,privado);
	//empaquetar id emisor
	pack(bufferOut,idEmisor);
	//empaquetar mensaje
	pack(bufferOut,msg.size());
	packv(bufferOut,msg.data(),msg.size());

	//buscar destinatario
	cerrojoClientes.lock();
	if(clientesConectados.find(idDestinatario)!=clientesConectados.end()){
		sendMSG(idDestinatario,bufferOut);
	}
	cerrojoClientes.unlock();
}

