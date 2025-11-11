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

            servidoresRegistrados[connectionId] = info;

            cout << "[BROKER] Servidor registrado - ID: " << connectionId
                 << " IP: " << info.ip << ":" << info.port << endl;
            cout << "[BROKER] Total servidores: " << servidoresRegistrados.size() << endl;

            buffer.clear();
            pack(buffer, ACK_BROKER);
            sendMSG(connectionId, buffer);
            break;
        }

        case CLIENT_CONNECT: {
            int serverConnId = findServerWithLessClients();

            buffer.clear();
            pack(buffer, ACK_BROKER);

            if (serverConnId != -1) {
                ServerInfo& info = servidoresRegistrados[serverConnId];

                info.numClients++;

                cout << "[BROKER] Cliente solicita servidor - Asignado: "
                     << info.ip << ":" << info.port
                     << " (Clientes: " << info.numClients << ")" << endl;

                pack(buffer, (int)info.ip.size());
                packv(buffer, (unsigned char*)info.ip.data(), (int)info.ip.size());
                pack(buffer, info.port);
            } else {
                cout << "[BROKER] ERROR: No hay servidores disponibles" << endl;
                pack(buffer, (int)0); // IP vacía
                pack(buffer, (int)0); // Puerto 0
            }

            sendMSG(connectionId, buffer);
            break;
        }
        case ACK_BROKER: {
            if (servidoresRegistrados.find(connectionId) != servidoresRegistrados.end()) {
                servidoresRegistrados[connectionId].keepAliveCounter = 0;
                servidoresRegistrados[connectionId].alive = true;
                cout << "[BROKER] Keep-alive recibido del servidor ID: " << connectionId << endl;
            }

            buffer.clear();
            pack(buffer, ACK_BROKER);
            sendMSG(connectionId, buffer);
            break;
        }

        default:
            cout << "[BROKER] Tipo de mensaje desconocido" << endl;
            break;
    }


    if (servidoresRegistrados.find(connectionId) != servidoresRegistrados.end()) {
        cout << "[BROKER] Servidor ID: " << connectionId << " desconectado" << endl;
        servidoresRegistrados.erase(connectionId);
        cout << "[BROKER] Total servidores: " << servidoresRegistrados.size() << endl;
    }

    closeConnection(connectionId);
}

int brokerManager::findServerWithLessClients() {
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

        vector<int> serverIds;
        for (auto& pair : servidoresRegistrados) {
            serverIds.push_back(pair.first);
        }

        for (int serverId : serverIds) {
            if (servidoresRegistrados.find(serverId) == servidoresRegistrados.end()) {
                continue;
            }

            servidoresRegistrados[serverId].keepAliveCounter++;

            if (servidoresRegistrados[serverId].keepAliveCounter > KEEP_ALIVE_MAX_COUNT) {
                if (servidoresRegistrados[serverId].alive) {
                    cout << "[BROKER] TIMEOUT: Servidor ID " << serverId
                         << " marcado como no disponible (sin keep-alive)" << endl;
                    servidoresRegistrados[serverId].alive = false;
                }
            }
        }
    }
}
