//
// Created by Rodrigo on 14/11/2025.
//

#include "brokerManager.h"
#include <iostream>

void brokerManager::resolveBrokerMessages(int idConnection) {
    vector<unsigned char> buffer;

    recvMSG(idConnection, buffer);
    if (buffer.empty()) {
        //ERROR
        closeConnection(idConnection);
        return;
    }
    brokerMsgTypes msgType = unpack<brokerMsgTypes>(buffer);
    switch (msgType) {
        case SERVER_CONNECT: {
            //estructura
            ServerInfo info;
            info.ip.resize(unpack<int>(buffer));
            unpackv(buffer, (char*)info.ip.data(), info.ip.size());
            info.port = unpack<int>(buffer);
            //Se puede añadir el numClientes o el keep alive counter
            {
                lock_guard<mutex> lock (serversMutex);
                servidoresRegistrados[idConnection] = info;
            }
            buffer.clear();
            pack(buffer, ACK_BROKER);
            sendMSG(idConnection, buffer);
            closeConnection(idConnection);
        }break;
        case CLIENT_CONNECT: {
            if (servidoresRegistrados.size()==0) {
                //Sería crear un error enum pero para simplificar así
                buffer.clear();
                pack(buffer, ACK_BROKER);
                pack(buffer, (int)0); //tamaño 0 ip
                pack(buffer, (int)0); //puerto 0
                sendMSG(idConnection, buffer);
                closeConnection(idConnection);
                break;
            }
            int servidorAsignado = rand() % servidoresRegistrados.size();
            buffer.clear();
            pack(buffer, ACK_BROKER);
            {
                lock_guard<mutex> lock(serversMutex);
                ServerInfo& info = servidoresRegistrados[servidorAsignado];
                pack(buffer, (int)info.ip.size());
                packv(buffer, (unsigned char *)info.ip.data(), (int)info.ip.size());
                pack(buffer, info.port);
                sendMSG(idConnection, buffer);
                closeConnection(idConnection);
            }
        }break;
        case ACK_BROKER: {
            //esto sería solo para el keep alive la verdad.

        }

    }
}