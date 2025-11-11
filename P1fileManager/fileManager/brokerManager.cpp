/*Rodrigo Fernández
* 07/11/2025
 */

#include "brokerManager.h"
#include <iostream>
#include <climits>

#define KEEP_ALIVE_MAX_COUNT 2

void brokerManager::resolveBrokerMessages(int connectionId) {
    vector<unsigned char> buffer;

    // Recibir mensaje
    recvMSG(connectionId, buffer);

    // Si el buffer está vacío, hubo un error de conexión
    if (buffer.empty()) {
        lock_guard<mutex> lock(serversMutex);
        if (servidoresRegistrados.find(connectionId) != servidoresRegistrados.end()) {
            cout << "[BROKER] Servidor ID: " << connectionId << " desconectado (error de red)" << endl;
            servidoresRegistrados.erase(connectionId);
            cout << "[BROKER] Total servidores: " << servidoresRegistrados.size() << endl;
        }
        closeConnection(connectionId);
        return;
    }

    brokerMsgTypes type = unpack<brokerMsgTypes>(buffer);

    switch(type) {
        case SERVER_CONNECT: {
            ServerInfo info;
            info.ip.resize(unpack<int>(buffer));
            unpackv(buffer, (char*)info.ip.data(), info.ip.size());
            info.port = unpack<int>(buffer);
            info.numClients = 0;
            info.keepAliveCounter = 0;
            info.alive = true;

            {
                lock_guard<mutex> lock(serversMutex);
                servidoresRegistrados[connectionId] = info;
                cout << "[BROKER] Servidor registrado - ID: " << connectionId
                     << " IP: " << info.ip << ":" << info.port << endl;
                cout << "[BROKER] Total servidores: " << servidoresRegistrados.size() << endl;
            }

            buffer.clear();
            pack(buffer, ACK_BROKER);
            sendMSG(connectionId, buffer);

            // Cerrar - servidor se reconecta para keep-alive
            closeConnection(connectionId);
            break;
        }

        case CLIENT_CONNECT: {
            int serverConnId = findServerWithLessClients();

            buffer.clear();
            pack(buffer, ACK_BROKER);

            if (serverConnId != -1) {
                lock_guard<mutex> lock(serversMutex);
                if (servidoresRegistrados.find(serverConnId) != servidoresRegistrados.end()) {
                    ServerInfo& info = servidoresRegistrados[serverConnId];
                    info.numClients++;

                    cout << "[BROKER] Cliente solicita servidor - Asignado: "
                         << info.ip << ":" << info.port
                         << " (Clientes: " << info.numClients << ")" << endl;

                    pack(buffer, (int)info.ip.size());
                    packv(buffer, (unsigned char*)info.ip.data(), (int)info.ip.size());
                    pack(buffer, info.port);
                } else {
                    cout << "[BROKER] ERROR: Servidor seleccionado ya no existe" << endl;
                    pack(buffer, (int)0);
                    pack(buffer, (int)0);
                }
            } else {
                cout << "[BROKER] ERROR: No hay servidores disponibles" << endl;
                pack(buffer, (int)0);
                pack(buffer, (int)0);
            }

            sendMSG(connectionId, buffer);

            // Cerrar - cliente solo consulta
            closeConnection(connectionId);
            break;
        }

        case ACK_BROKER: {
            {
                lock_guard<mutex> lock(serversMutex);
                if (servidoresRegistrados.find(connectionId) != servidoresRegistrados.end()) {
                    servidoresRegistrados[connectionId].keepAliveCounter = 0;
                    servidoresRegistrados[connectionId].alive = true;
                    cout << "[BROKER] Keep-alive recibido del servidor ID: " << connectionId << endl;
                }
            }

            buffer.clear();
            pack(buffer, ACK_BROKER);
            sendMSG(connectionId, buffer);

            // Cerrar - keep-alive temporal
            closeConnection(connectionId);
            break;
        }

        default:
            cout << "[BROKER] Tipo de mensaje desconocido" << endl;
            closeConnection(connectionId);
            break;
    }
}

int brokerManager::findServerWithLessClients() {
    lock_guard<mutex> lock(serversMutex);

    if (servidoresRegistrados.empty()) {
        return -1;
    }

    int minClients = INT_MAX;
    int selectedServer = -1;

    for (auto& pair : servidoresRegistrados) {
        if (pair.second.alive && pair.second.numClients < minClients) {
            minClients = pair.second.numClients;
            selectedServer = pair.first;
        }
    }

    return selectedServer;
}

void brokerManager::keepAliveMonitor() {
    while(true) {
        sleep(5);

        lock_guard<mutex> lock(serversMutex);

        for (auto& pair : servidoresRegistrados) {
            pair.second.keepAliveCounter++;

            if (pair.second.keepAliveCounter > KEEP_ALIVE_MAX_COUNT) {
                if (pair.second.alive) {
                    cout << "[BROKER] TIMEOUT: Servidor ID " << pair.first
                         << " (" << pair.second.ip << ":" << pair.second.port
                         << ") marcado como no disponible" << endl;
                    pair.second.alive = false;
                }
            }
        }
    }
}
