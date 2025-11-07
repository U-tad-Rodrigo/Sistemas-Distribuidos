/*Rodrigo Fernández
* 05/11/2205
 */

#include "clientManager.h"
#include "brokerManager.h"
#include "utils.h"
#include <iostream>
#include <string>
#include <thread>
#include <list>

using namespace std;

#define BROKER_IP "127.0.0.1"
#define BROKER_PORT 1066
#define SERVER_PORT 1067
#define SERVER_IP "127.0.0.1"
#define KEEP_ALIVE_INTERVAL 5 // segundos

int brokerConnectionId = -1;

void sendKeepAlive() {
    while(true) {
        sleep(KEEP_ALIVE_INTERVAL);

        if (brokerConnectionId != -1) {
            vector<unsigned char> buffer;
            pack(buffer, ACK_BROKER); // Keep-alive es un ACK_BROKER
            sendMSG(brokerConnectionId, buffer);

            // Esperar respuesta
            recvMSG(brokerConnectionId, buffer);
            if (unpack<brokerMsgTypes>(buffer) == ACK_BROKER) {
                cout << "[SERVER] Keep-alive enviado al broker" << endl;
            }
        }
    }
}

int main(int argc, char** argv)
{
 bool exit = false;

 cout << "[SERVER] Conectando con broker en " << BROKER_IP << ":" << BROKER_PORT << endl;
 connection_t brokerConn = initClient(BROKER_IP, BROKER_PORT);
 brokerConnectionId = brokerConn.serverId;

 vector<unsigned char> buffer;
 pack(buffer, SERVER_CONNECT);

 string serverIp = SERVER_IP;
 pack(buffer, (int)serverIp.size());
 packv(buffer, (unsigned char*)serverIp.data(), (int)serverIp.size());
 pack(buffer, SERVER_PORT);

 sendMSG(brokerConnectionId, buffer);

 // Esperar ACK
 recvMSG(brokerConnectionId, buffer);
 if (unpack<brokerMsgTypes>(buffer) == ACK_BROKER) {
     cout << "[SERVER] Registrado en broker correctamente" << endl;
 } else {
     cout << "[SERVER] ERROR: No se pudo registrar en broker" << endl;
     return -1;
 }

 thread keepAliveThread(sendKeepAlive);
 keepAliveThread.detach();
 cout << "[SERVER] Thread keep-alive iniciado" << endl;

 cout << "[SERVER] Abriendo puerto " << SERVER_PORT << " para clientes..." << endl;
 int serverPortId = initServer(SERVER_PORT);
 cout << "[SERVER] Servidor listo, esperando conexiones" << endl;

 while(!exit) {
  while (!checkClient()) usleep(100);
  int clientId = getLastClientID();
  cout << "[SERVER] Cliente ID: " << clientId << " conectado" << endl;
  auto* th = new thread(clientManager::resolveClientMessages, clientId);
  th->detach();
 }

 return 0;
}
