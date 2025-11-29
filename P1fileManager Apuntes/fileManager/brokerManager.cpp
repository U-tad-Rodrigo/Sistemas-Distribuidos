/*Rodrigo Fernández
* 07/11/2025
 */

#include "brokerManager.h"
#include <iostream>
#include <climits>

// CONFIGURACIÓN: Número máximo de ciclos sin keep-alive antes de marcar servidor como caído
// Ejemplo: Si el monitor corre cada 5 seg y MAX=2, tras 10 seg sin respuesta -> servidor muerto
#define KEEP_ALIVE_MAX_COUNT 2

// ==================== FUNCIÓN PRINCIPAL: RESOLVER MENSAJES DEL BROKER ====================
// CONCEPTO: Esta función corre en un thread separado para CADA conexión entrante
// PARÁMETRO: connectionId = identificador único de esta conexión específica
// IMPORTANTE: Puede haber múltiples ejecuciones de esta función en paralelo
void brokerManager::resolveBrokerMessages(int connectionId) {
    vector<unsigned char> buffer;

    // ==================== PASO 1: RECIBIR MENSAJE ====================
    // recvMSG() bloquea hasta recibir datos completos del socket
    // PROTOCOLO: Primero lee tamaño del mensaje, luego lee los datos
    // buffer se llena con bytes que luego serán "desempaquetados"
    recvMSG(connectionId, buffer);

    // ==================== MANEJO DE DESCONEXIONES ====================
    // Buffer vacío = el socket se cerró o hubo error de red
    // IMPORTANTE: Limpiar el registro si era un servidor conocido
    if (buffer.empty()) {
        lock_guard<mutex> lock(serversMutex);  // Proteger acceso al mapa compartido
        if (servidoresRegistrados.find(connectionId) != servidoresRegistrados.end()) {
            cout << "[BROKER] Servidor ID: " << connectionId << " desconectado (error de red)" << endl;
            servidoresRegistrados.erase(connectionId);  // Eliminar del registro
            cout << "[BROKER] Total servidores: " << servidoresRegistrados.size() << endl;
        }
        closeConnection(connectionId);
        return;
    }

    // ==================== PASO 2: IDENTIFICAR TIPO DE MENSAJE ====================
    // PROTOCOLO: El primer campo siempre es el tipo de mensaje (enum)
    // unpack() extrae datos del buffer y los elimina (buffer se reduce)
    brokerMsgTypes type = unpack<brokerMsgTypes>(buffer);

    // ==================== SWITCH: MANEJAR CADA TIPO DE MENSAJE ====================
    switch(type) {
        // ==================== CASO 1: REGISTRO DE SERVIDOR ====================
        // FLUJO: Un servidor quiere anunciarse al broker para recibir clientes
        case SERVER_CONNECT: {
            ServerInfo info;

            // DESEMPAQUETADO: IP como string (primero su tamaño, luego los caracteres)
            info.ip.resize(unpack<int>(buffer));  // Leer tamaño del string
            unpackv(buffer, (unsigned char*)info.ip.data(), info.ip.size());  // Leer caracteres

            // Puerto donde el servidor acepta conexiones de clientes
            info.port = unpack<int>(buffer);

            // INICIALIZACIÓN: Estado inicial del servidor
            info.numClients = 0;           // Sin clientes al inicio
            info.keepAliveCounter = 0;     // Contador de keep-alive a cero
            info.alive = true;             // Servidor disponible desde el inicio

            // ==================== SECCIÓN CRÍTICA ====================
            // CONCURRENCIA: lock_guard bloquea el mutex automáticamente
            // Garantiza que solo un thread modifica servidoresRegistrados a la vez
            // Al salir del bloque {}, el mutex se desbloquea automáticamente
            {
                lock_guard<mutex> lock(serversMutex);
                servidoresRegistrados[connectionId] = info;  // Registrar servidor
                cout << "[BROKER] Servidor registrado - ID: " << connectionId
                     << " IP: " << info.ip << ":" << info.port << endl;
                cout << "[BROKER] Total servidores: " << servidoresRegistrados.size() << endl;
            }

            // ENVIAR CONFIRMACIÓN (ACK)
            buffer.clear();
            pack(buffer, ACK_BROKER);  // Empaquetar tipo de mensaje
            sendMSG(connectionId, buffer);  // Enviar por socket

            // ==================== IMPORTANTE: CERRAR CONEXIÓN ====================
            // DISEÑO: Conexiones efímeras - el servidor se reconectará para keep-alive
            // VENTAJA: No mantiene sockets abiertos innecesariamente
            // NOTA: connectionId queda registrado en el mapa aunque se cierre el socket
            closeConnection(connectionId);
            break;
        }

        // ==================== CASO 2: CLIENTE SOLICITA SERVIDOR ====================
        // FLUJO: Un cliente pide al broker que le asigne un servidor disponible
        case CLIENT_CONNECT: {
            // BALANCEO DE CARGA: Encontrar servidor con menos clientes
            int serverConnId = findServerWithLessClients();

            buffer.clear();
            pack(buffer, ACK_BROKER);  // Siempre responder con ACK

            if (serverConnId != -1) {
                // SERVIDOR ENCONTRADO
                lock_guard<mutex> lock(serversMutex);
                if (servidoresRegistrados.find(serverConnId) != servidoresRegistrados.end()) {
                    ServerInfo& info = servidoresRegistrados[serverConnId];
                    info.numClients++;  // Incrementar contador de clientes

                    cout << "[BROKER] Cliente solicita servidor - Asignado: "
                         << info.ip << ":" << info.port
                         << " (Clientes: " << info.numClients << ")" << endl;

                    // EMPAQUETAR RESPUESTA: IP + Puerto del servidor
                    pack(buffer, (int)info.ip.size());
                    packv(buffer, (unsigned char*)info.ip.data(), (int)info.ip.size());
                    pack(buffer, info.port);
                } else {
                    // ERROR: El servidor seleccionado desapareció entre búsqueda y asignación
                    cout << "[BROKER] ERROR: Servidor seleccionado ya no existe" << endl;
                    pack(buffer, (int)0);  // Indicar error con tamaño 0
                    pack(buffer, (int)0);
                }
            } else {
                // NO HAY SERVIDORES DISPONIBLES
                cout << "[BROKER] ERROR: No hay servidores disponibles" << endl;
                pack(buffer, (int)0);  // Tamaño 0 = sin servidor
                pack(buffer, (int)0);
            }

            sendMSG(connectionId, buffer);

            // CERRAR: El cliente solo consultaba, no mantiene conexión con broker
            closeConnection(connectionId);
            break;
        }

        // ==================== CASO 3: KEEP-ALIVE (HEARTBEAT) ====================
        // FLUJO: Un servidor envía señal de vida periódica
        // CONCEPTO: Detección de fallos mediante heartbeats
        case ACK_BROKER: {
            {
                // IMPORTANTE: El servidor envía su ID original (del registro)
                int ogServerId = unpack<int>(buffer);
                lock_guard<mutex> lock(serversMutex);
                if (servidoresRegistrados.find(ogServerId) != servidoresRegistrados.end()) {
                    // RESETEAR CONTADOR: El servidor está vivo
                    servidoresRegistrados[ogServerId].keepAliveCounter = 0;
                    servidoresRegistrados[ogServerId].alive = true;
                }
            }

            // RESPONDER CON ACK (confirmar recepción del keep-alive)
            buffer.clear();
            pack(buffer, ACK_BROKER);
            sendMSG(connectionId, buffer);

            // CERRAR: Keep-alive es temporal, no mantiene conexión
            closeConnection(connectionId);
            break;
        }

        default:
            cout << "[BROKER] Tipo de mensaje desconocido" << endl;
            closeConnection(connectionId);
            break;
    }
}

// ==================== BALANCEO DE CARGA: ENCONTRAR SERVIDOR ÓPTIMO ====================
// ESTRATEGIA: Asignar clientes al servidor con menos carga
// CONCEPTO: Load Balancing - distribuir carga equitativamente entre servidores
// RETORNA: connectionId del servidor seleccionado, o -1 si no hay disponibles
int brokerManager::findServerWithLessClients() {
    lock_guard<mutex> lock(serversMutex);  // Proteger lectura del mapa

    if (servidoresRegistrados.empty()) {
        return -1;  // No hay servidores registrados
    }

    int minClients = INT_MAX;  // Empezar con valor máximo
    int selectedServer = -1;

    // ITERAR: Buscar servidor con mínimo número de clientes
    for (auto& pair : servidoresRegistrados) {
        // CONDICIONES: Debe estar vivo Y tener menos clientes que el actual mínimo
        if (pair.second.alive && pair.second.numClients < minClients) {
            minClients = pair.second.numClients;
            selectedServer = pair.first;  // Guardar connectionId
        }
    }

    return selectedServer;
}

// ==================== MONITOR DE KEEP-ALIVE (DETECCIÓN DE FALLOS) ====================
// CONCEPTO: Thread que corre continuamente verificando la salud de los servidores
// MECANISMO: Heartbeat/Keep-alive - si un servidor no envía señales -> está caído
// IMPORTANTE: Esta función NUNCA termina (bucle infinito)
void brokerManager::keepAliveMonitor() {
    while(true) {
        sleep(5);  // Esperar 5 segundos entre cada verificación

        lock_guard<mutex> lock(serversMutex);  // Proteger acceso al mapa

        // ITERAR: Verificar cada servidor registrado
        for (auto& pair : servidoresRegistrados) {
            // INCREMENTAR CONTADOR: Cada ciclo suma 1
            pair.second.keepAliveCounter++;

            // VERIFICAR TIMEOUT: Si el contador supera el máximo permitido
            if (pair.second.keepAliveCounter > KEEP_ALIVE_MAX_COUNT) {
                if (pair.second.alive) {
                    // MARCAR COMO CAÍDO: No eliminar del mapa, solo marcar como no disponible
                    // VENTAJA: Si el servidor vuelve, puede reactivarse con keep-alive
                    cout << "[BROKER] TIMEOUT: Servidor ID " << pair.first
                         << " (" << pair.second.ip << ":" << pair.second.port
                         << ") marcado como no disponible" << endl;
                    pair.second.alive = false;
                }
            }
        }
        // NOTA: El contador se resetea a 0 cuando el servidor envía keep-alive (caso ACK_BROKER)
    }
}
