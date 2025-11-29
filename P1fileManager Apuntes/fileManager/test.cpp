//
// Created by Rodrigo on 14/11/2025.
//

//Main de prueba para estudiar

#include <iostream>
#include "clientManager.h"
#include "brokerManager.h"
#include <thread>
#include <string>

//primero definimos el ip y puerto del server y broker

#define BROKER_IP "127.0.0.1"
#define BROKER_PORT 1066
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 1067
using namespace std;

//tenemos que guardar el id del broken para el keep alive, pero este
//está sin keep alive

int main(int argc, char* argv[]) {
    bool exit = false;

    connection_t brokerConnection = initClient(BROKER_IP, BROKER_PORT);
    vector<unsigned char> buffer;
    pack(buffer, SERVER_CONNECT);
    string ipServidor = SERVER_IP;
    pack(buffer, (int)ipServidor.size());
    packv(buffer, (unsigned char*)ipServidor.data(), (int)ipServidor.size());
    pack(buffer, SERVER_PORT);
    sendMSG(brokerConnection.serverId, buffer);

    recvMSG(brokerConnection.serverId, buffer);

    int idServidorPuerto = initServer(SERVER_PORT);

    while (!exit) {
        while (!checkClient()) usleep(100);
        int idCliente = getLastClientID();
        auto* th = new thread(clientManager::resolveClientMessages, idCliente);
        th->detach();
    }
    return 0;
}