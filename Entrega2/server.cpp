/*Rodrigo Fernández
 * 05/11/2025
 */

#include "clientManager.h"
#include "utils.h"
#include <iostream>
#include <thread>
#include <signal.h>

using namespace std;

bool servidorActivo = true;

void manejarSenalApagado(int signum) {
	cout << "\n[SERVIDOR] Recibida señal de apagado (Ctrl+C)" << endl;
	cout << "[SERVIDOR] Enviando mensaje de cierre a clientes..." << endl;

	servidorActivo = false;

	//enviar mensaje de cierre a todos los clientes
	clientManager::cerrojoClientes.lock();
	for(auto client : clientManager::clientesConectados){
		vector<unsigned char> buffer;
		pack(buffer, clientManager::exit);
		sendMSG(client.second, buffer);
	}
	clientManager::cerrojoClientes.unlock();

	cout << "[SERVIDOR] Esperando que los clientes se desconecten..." << endl;
	sleep(2);

	cout << "[SERVIDOR] Apagado completado." << endl;
	exit(0);
}

int main(int argc, char** argv) {
	int puerto = 5000;

	if (argc >= 2) {
		puerto = atoi(argv[1]);
		if (puerto <= 0 || puerto > 65535) {
			cerr << "Puerto invalido. Usando puerto por defecto: 5000" << endl;
			puerto = 5000;
		}
	}

	//configurar manejo de señal SIGINT (Ctrl+C)
	signal(SIGINT, manejarSenalApagado);

	cout << "Servidor TCP Multi-Cliente" << endl;
	cout << "Puerto: " << puerto << endl;
	cout << "Presiona Ctrl+C para apagar" << endl;
	cout << "----------------------------" << endl;

	int serverPortId = initServer(puerto);
	cout << "Servidor iniciado, esperando conexiones..." << endl;

	while (servidorActivo) {
		while (!checkClient() && servidorActivo) usleep(100);
		if(!servidorActivo) break;

		int clientId = getLastClientID();
		cout << "[SERVIDOR] Cliente " << clientId << " conectado" << endl;

		thread* th = new thread(clientManager::atiendeCliente, clientId);
		th->detach();
	}

	return 0;
}