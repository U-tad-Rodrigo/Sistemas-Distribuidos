/*Rodrigo Fernández
* 07/11/2025
 */

#include "brokerManager.h"
#include "utils.h"
#include <iostream>
#include <thread>

using namespace std;

#define BROKER_PORT 1066

int main(int argc, char** argv)
{
    cout << "Puerto: " << BROKER_PORT << endl;
    cout << "Iniciando broker..." << endl;

    int brokerPortId = initServer(BROKER_PORT);

    cout << "Broker activo y esperando conexiones" << endl;

    thread keepAliveThread(brokerManager::keepAliveMonitor);
    keepAliveThread.detach();
    cout << "[BROKER] Thread keep-alive iniciado" << endl << endl;

    // Loop principal: aceptar conexiones
    while(true) {
        while (!checkClient()) usleep(100);

        int connectionId = getLastClientID();
        cout << "\n[BROKER] Nueva conexión ID: " << connectionId << endl;

        // Crear thread para manejar esta conexión
        thread* th = new thread(brokerManager::resolveBrokerMessages, connectionId);
        th->detach();
    }

    return 0;
}

