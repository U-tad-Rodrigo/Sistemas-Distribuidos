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

// VARIABLE GLOBAL: Guardar el ID original del registro para los keep-alives
// IMPORTANTE: El servidor se registra una vez, pero envía múltiples keep-alives
// Cada keep-alive usa una conexión nueva pero referencia el ID original
unsigned int brokerConnectionId = -1;

// ==================== THREAD DE KEEP-ALIVE ====================
// CONCEPTO: Heartbeat - el servidor envía señales periódicas al broker
// OBJETIVO: Informar al broker que este servidor sigue vivo y disponible
// IMPORTANTE: Corre en un thread separado, independiente del procesamiento de clientes
void sendKeepAlive() {
    while(true) {
        sleep(KEEP_ALIVE_INTERVAL);  // Esperar intervalo configurado

        // NUEVA CONEXIÓN: Cada keep-alive abre y cierra conexión (efímero)
        // ALTERNATIVA: Podría mantener conexión persistente, pero esto es más simple
        connection_t brokerConn = initClient(BROKER_IP, BROKER_PORT);

        // EMPAQUETAR MENSAJE: Tipo ACK_BROKER + ID original del servidor
        vector<unsigned char> buffer;
        pack(buffer, ACK_BROKER);
        pack(buffer, brokerConnectionId);  // CRUCIAL: Enviar ID del registro original

        // ENVIAR Y ESPERAR CONFIRMACIÓN
        sendMSG(brokerConn.serverId, buffer);
        buffer.clear();
        recvMSG(brokerConn.serverId, buffer);  // Esperar ACK del broker

        // CERRAR: Conexión temporal completada
        closeConnection(brokerConn.serverId);

        // CICLO: Espera KEEP_ALIVE_INTERVAL y repite indefinidamente
    }
}

// ==================== MAIN DEL SERVIDOR ====================
// ARQUITECTURA: El servidor tiene dos roles:
// 1) Cliente del broker (para registrarse y enviar keep-alive)
// 2) Servidor para clientes finales (procesa sus peticiones)
int main(int argc, char** argv)
{
 bool exit = false;

 // ==================== FASE 1: REGISTRO EN EL BROKER ====================
 // CONCEPTO: Service Discovery - anunciarse para que el broker nos conozca
 cout << "[SERVER] Conectando con broker en " << BROKER_IP << ":" << BROKER_PORT << endl;
 brokerConnectionId = initClient(BROKER_IP, BROKER_PORT).serverId;
  // GUARDAR: ID para futuros keep-alives

 // CONSTRUIR MENSAJE DE REGISTRO
 vector<unsigned char> buffer;
 pack(buffer, SERVER_CONNECT);  // Tipo de mensaje

 // EMPAQUETAR INFORMACIÓN DEL SERVIDOR: IP y Puerto donde aceptamos clientes
 string serverIp = SERVER_IP;
 pack(buffer, (int)serverIp.size());  // Primero el tamaño del string
 packv(buffer, (unsigned char*)serverIp.data(), (int)serverIp.size());  // Luego los datos
 pack(buffer, SERVER_PORT);  // Puerto donde los clientes se conectarán

 // ENVIAR REGISTRO
 sendMSG(brokerConnectionId, buffer);

 // ==================== ESPERAR CONFIRMACIÓN ====================
 // PROTOCOLO: Comunicación síncrona - esperar ACK antes de continuar
 recvMSG(brokerConnectionId, buffer);
 if (unpack<brokerMsgTypes>(buffer) == ACK_BROKER) {
     cout << "[SERVER] Registrado en broker correctamente" << endl;
 } else {
     cout << "[SERVER] ERROR: No se pudo registrar en broker" << endl;
     return -1;  // FALLO CRÍTICO: Sin registro, no podemos recibir clientes
 }

 // NOTA: La conexión con el broker se cierra automáticamente por el broker
 // tras enviar el ACK (diseño de conexiones efímeras)

 // ==================== FASE 2: INICIAR KEEP-ALIVE ====================
 // CONCEPTO: Detección de fallos - señales periódicas de vida
 thread keepAliveThread(sendKeepAlive);
 keepAliveThread.detach();  // No esperamos a que termine (corre para siempre)
 cout << "[SERVER] Thread keep-alive iniciado" << endl;

 // ==================== FASE 3: ATENDER CLIENTES ====================
 // CONCEPTO: Ahora actuamos como servidor para los clientes finales
 cout << "[SERVER] Abriendo puerto " << SERVER_PORT << " para clientes..." << endl;
 int serverPortId = initServer(SERVER_PORT);
 cout << "[SERVER] Servidor listo, esperando conexiones" << endl;

 // BUCLE PRINCIPAL: Aceptar y procesar conexiones de clientes
 // ARQUITECTURA: Un thread por cliente para concurrencia
 while(!exit) {
  // ESPERA ACTIVA: Verificar si hay nuevos clientes
  while (!checkClient()) usleep(100);

  // NUEVA CONEXIÓN: Obtener ID del cliente conectado
  int clientId = getLastClientID();
  cout << "[SERVER] Cliente ID: " << clientId << " conectado" << endl;

  // THREAD POR CLIENTE: Permite atender múltiples clientes simultáneamente
  // IMPORTANTE: new thread = memoria dinámica, detach() = se autodestruye
  auto* th = new thread(clientManager::resolveClientMessages, clientId);
  th->detach();

  // FLUJO: El main vuelve a esperar nuevos clientes mientras los threads
  // procesan las peticiones de cada cliente en paralelo
 }

 return 0;
}
