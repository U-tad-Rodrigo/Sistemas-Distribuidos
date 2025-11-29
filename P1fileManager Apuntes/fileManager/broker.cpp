/*Rodrigo Fernández
* 07/11/2025
 */

#include "brokerManager.h"
#include "utils.h"
#include <iostream>
#include <thread>

using namespace std;

#define BROKER_PORT 1066

// ==================== MAIN DEL BROKER ====================
// CONCEPTO: El broker es el punto central de descubrimiento de servicios
// ARQUITECTURA: Patrón Mediador + Registro de Servicios
// FLUJO: 1) Abre puerto, 2) Lanza thread de keep-alive, 3) Acepta conexiones en bucle
int main(int argc, char** argv)
{
    cout << "Puerto: " << BROKER_PORT << endl;
    cout << "Iniciando broker..." << endl;

    // ==================== PASO 1: ABRIR SOCKET SERVIDOR ====================
    // initServer() crea un socket TCP en modo escucha (listen)
    // Internamente hace: socket() -> bind() -> listen()
    // IMPORTANTE: Este socket solo acepta conexiones, no transmite datos
    int brokerPortId = initServer(BROKER_PORT);

    cout << "Broker activo y esperando conexiones" << endl;

    // ==================== PASO 2: THREAD DE MONITOREO (KEEP-ALIVE) ====================
    // CONCEPTO: Detección de fallos - los servidores deben enviar señales periódicas
    // Este thread corre en background verificando continuamente la salud de los servidores
    // detach() = el thread corre independientemente del main, no esperamos su finalización
    thread keepAliveThread(brokerManager::keepAliveMonitor);
    keepAliveThread.detach();
    cout << "[BROKER] Thread keep-alive iniciado" << endl << endl;

    // ==================== PASO 3: BUCLE PRINCIPAL - ACEPTAR CONEXIONES ====================
    // PATRÓN: Reactor/Event Loop - espera eventos (nuevas conexiones) y los procesa
    // IMPORTANTE: Este bucle NUNCA termina (servidor siempre activo)
    while(true) {
        // Espera activa: checkClient() verifica si hay conexiones pendientes
        // usleep(100) = duerme 100 microsegundos para no consumir 100% CPU
        // ALTERNATIVA: Podría usar select() o epoll() para espera bloqueante
        while (!checkClient()) usleep(100);

        // Nueva conexión detectada - obtener su ID único
        // El ID es asignado automáticamente por utils.cpp (contador incremental)
            int connectionId = getLastClientID();
        cout << "\n[BROKER] Nueva conexión ID: " << connectionId << endl;

        // ==================== PROCESAMIENTO CONCURRENTE ====================
        // CONCEPTO: Un thread por conexión = el broker puede atender múltiples
        // clientes/servidores simultáneamente sin bloquearse
        // IMPORTANTE: new thread = memoria dinámica, detach() = se autodestruye al terminar
        thread* th = new thread(brokerManager::resolveBrokerMessages, connectionId);
        th->detach();  // No esperamos a que termine, puede correr en paralelo

        // NOTA: Cada thread ejecuta resolveBrokerMessages() con su propio connectionId
        // Esto permite que el broker maneje múltiples solicitudes al mismo tiempo
    }

    return 0;
}

