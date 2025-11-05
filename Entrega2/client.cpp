/*Rodrigo Fernández
 * 05/11/2025
 */

#include "utils.h"
#include "clientManager.h"
#include <iostream>
#include <string>
#include <thread>

using namespace std;

void enviaMensaje(int id, string mensaje)
{
	vector<unsigned char> buffer;
	//empaquetar tipo
	pack(buffer,clientManager::texto);
	//empaquetar mensaje
	pack(buffer,mensaje.size());
	packv(buffer,mensaje.data(),mensaje.size());
	//enviar
	sendMSG(id,buffer);
	//esperar ack
	buffer.clear();
	recvMSG(id,buffer);
	if(unpack<int>(buffer)!=clientManager::ack)
		cout<<"Error enviando mensaje\n";
}

void enviaComandoUsuarios(int id)
{
	vector<unsigned char> buffer;
	//empaquetar tipo
	pack(buffer,clientManager::usuarios);
	//enviar
	sendMSG(id,buffer);
	//esperar respuesta
	buffer.clear();
	recvMSG(id,buffer);
	clientManager::msgTypes type=unpack<clientManager::msgTypes>(buffer);
	if(type==clientManager::usuarios){
		string lista;
		lista.resize(unpack<long int>(buffer));
		unpackv(buffer,(char*)lista.data(),lista.size());
		cout<<lista<<endl;
	}
	//esperar ack
	buffer.clear();
	recvMSG(id,buffer);
}

void enviaMensajePrivado(int id, int destinatario, string mensaje)
{
	vector<unsigned char> buffer;
	//empaquetar tipo
	pack(buffer,clientManager::privado);
	//empaquetar destinatario
	pack(buffer,destinatario);
	//empaquetar mensaje
	pack(buffer,mensaje.size());
	packv(buffer,mensaje.data(),mensaje.size());
	//enviar
	sendMSG(id,buffer);
	//esperar ack
	buffer.clear();
	recvMSG(id,buffer);
	if(unpack<int>(buffer)!=clientManager::ack)
		cout<<"Error enviando mensaje privado\n";
}

void enviaExit(int id)
{
	vector<unsigned char> buffer;
	//empaquetar tipo
	pack(buffer,clientManager::exit);
	//enviar
	sendMSG(id,buffer);
	//esperar ack
	buffer.clear();
	recvMSG(id,buffer);
}

void recibeMensajes(int serverId, bool &conectado)
{
	vector<unsigned char> buffer;
	while(conectado){
		//recibir mensaje
		recvMSG(serverId,buffer);
		//desempaquetar tipo
		clientManager::msgTypes type=unpack<clientManager::msgTypes>(buffer);

		switch(type){
			case clientManager::texto:{
				//desempaquetar id emisor
				int idEmisor=unpack<int>(buffer);
				//desempaquetar mensaje
				string msg;
				msg.resize(unpack<long int>(buffer));
				unpackv(buffer,(char*)msg.data(),msg.size());
				cout<<"cliente "<<idEmisor<<": "<<msg<<endl;
			}break;

			case clientManager::privado:{
				//desempaquetar id emisor
				int idEmisor=unpack<int>(buffer);
				//desempaquetar mensaje
				string msg;
				msg.resize(unpack<long int>(buffer));
				unpackv(buffer,(char*)msg.data(),msg.size());
				cout<<"[PRIVADO] cliente "<<idEmisor<<": "<<msg<<endl;
			}break;

			default:
				break;
		}
		buffer.clear();
	}
}

int main(int argc, char** argv) {
	string host = "127.0.0.1";
	int puerto = 5000;

	if (argc >= 2) {
		host = argv[1];
	}
	if (argc >= 3) {
		puerto = atoi(argv[2]);
		if (puerto <= 0 || puerto > 65535) {
			cerr << "Puerto invalido. Usando puerto por defecto: 5000" << endl;
			puerto = 5000;
		}
	}

	cout << "========================================" << endl;
	cout << "       CLIENTE TCP INTERACTIVO" << endl;
	cout << "========================================" << endl;
	cout << "Conectando a " << host << ":" << puerto << "..." << endl;

	connection_t connection = initClient(host, puerto);

	if (!connection.alive) {
		cerr << "Error al conectar con el servidor" << endl;
		return 1;
	}

	int serverId = connection.serverId;

	cout << "Conexion establecida!" << endl;
	cout << "========================================" << endl;
	cout << "Comandos disponibles:" << endl;
	cout << "  - Escribe un mensaje para enviarlo" << endl;
	cout << "  - 'usuarios' para ver clientes conectados" << endl;
	cout << "  - 'exit' para desconectar" << endl;
	cout << "  - '@ID mensaje' para mensaje privado" << endl;
	cout << "========================================" << endl << endl;

	bool conectado = true;
	thread* hiloRecepcion = new thread(recibeMensajes, serverId, ref(conectado));

	string mensaje;
	while (conectado) {
		cout << "> ";
		if (!getline(cin, mensaje)) {
			break;
		}

		if (mensaje.empty()) {
			continue;
		}

		if (mensaje == "exit") {
			cout << "Cerrando conexion..." << endl;
			enviaExit(serverId);
			conectado = false;
			break;
		}
		else if (mensaje == "usuarios") {
			enviaComandoUsuarios(serverId);
		}
		else if (mensaje[0] == '@') {
			//mensaje privado
			size_t espacioPos = mensaje.find(' ');
			if (espacioPos != string::npos && espacioPos > 1) {
				string idStr = mensaje.substr(1, espacioPos - 1);
				string textoMensaje = mensaje.substr(espacioPos + 1);
				try {
					int idDestinatario = stoi(idStr);
					enviaMensajePrivado(serverId, idDestinatario, textoMensaje);
					cout << "Servidor: mensaje privado enviado a cliente " << idStr << endl;
				} catch (...) {
					cout << "Formato incorrecto. Usa: @ID mensaje" << endl;
				}
			} else {
				cout << "Formato incorrecto. Usa: @ID mensaje" << endl;
			}
		}
		else {
			enviaMensaje(serverId, mensaje);
			cout << "Servidor: mensaje recibido correctamente." << endl;
		}
	}

	conectado = false;
	closeConnection(serverId);

	if (hiloRecepcion->joinable()) {
		hiloRecepcion->join();
	}

	cout << "Desconectado." << endl;

	return 0;
}

