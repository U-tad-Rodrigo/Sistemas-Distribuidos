/*Rodrigo Fernández
 * 05/11/2025
 */

#include "clientManager.h"
#include "utils.h"
#include <iostream>
#include <thread>

using namespace std;

int main(int argc, char** argv) {
	bool exit = false;
	int puerto = 5000;

	if (argc >= 2) {
		puerto = atoi(argv[1]);
		if (puerto <= 0 || puerto > 65535) {
			cerr << "Puerto invalido. Usando puerto por defecto: 5000" << endl;
			puerto = 5000;
		}
	}

	cout << "============================================" << endl;
	cout << "  SERVIDOR TCP MULTI-CLIENTE CON BROADCAST" << endl;
	cout << "============================================" << endl;
	cout << "Puerto: " << puerto << endl;
	cout << "============================================" << endl;

	cout << "Servidor abriendo puerto..." << endl;
	int serverPortId = initServer(puerto);
	cout << "Puerto abierto, esperando conexiones..." << endl;
	cout << "============================================" << endl << endl;

	while (!exit) {
		while (!checkClient()) usleep(100);
		int clientId = getLastClientID();
		cout << "[SERVIDOR] Cliente ID: " << clientId << " conectado" << endl;

		thread* th = new thread(clientManager::atiendeCliente, clientId);
	}

	return 0;
}