/*Rodrigo Fernández
* 07/11/2025
 */
#pragma once

#include "utils.h"
#include <string>
#include <map>
#include <mutex>

using namespace std;

// ==================== TIPOS DE MENSAJES DEL BROKER ====================
// CONCEPTO CLAVE: El broker es un intermediario que conecta clientes con servidores
// Los mensajes definen las operaciones que el broker puede manejar
typedef enum{
    SERVER_CONNECT,    // Un servidor quiere registrarse en el broker
    CLIENT_CONNECT,    // Un cliente pide que el broker le asigne un servidor
    ACK_BROKER         // Confirmación del broker (también usado para keep-alive)
}brokerMsgTypes;

// ==================== ESTRUCTURA DE INFORMACIÓN DE SERVIDOR ====================
// CONCEPTO CLAVE: El broker mantiene un registro de todos los servidores disponibles
// Esta estructura almacena toda la información necesaria de cada servidor
typedef struct {
    string ip;                  // IP del servidor para que los clientes puedan conectarse
    int port;                   // Puerto del servidor para conexiones de clientes
    int numClients;             // BALANCEO DE CARGA: contador de clientes conectados
    int keepAliveCounter;       // DETECCIÓN DE FALLOS: contador de ciclos sin respuesta
    bool alive;                 // Estado del servidor (true = disponible, false = caído)
}ServerInfo;

// ==================== CLASE BROKER MANAGER ====================
// PATRÓN: Mediador/Broker - centraliza la comunicación entre clientes y servidores
// CONCEPTO: El broker no procesa peticiones de negocio, solo conecta componentes
class brokerManager {
public:
    // ==================== ESTADO COMPARTIDO ====================
    // IMPORTANTE: Variables "inline static" = compartidas entre todos los threads
    // La key del map es el connectionId (identificador único de cada servidor)
    static inline map<int, ServerInfo> servidoresRegistrados;

    // CONCURRENCIA: Mutex protege el mapa de accesos concurrentes por múltiples threads
    // Sin mutex, dos threads podrían modificar el mapa simultáneamente (condición de carrera)
    static inline mutex serversMutex;

    // ==================== MÉTODOS PRINCIPALES ====================

    // FUNCIÓN: Procesa mensajes entrantes al broker (corre en un thread por conexión)
    // PARÁMETRO: connectionId = identificador único de la conexión actual
    // MANEJA: SERVER_CONNECT (registro), CLIENT_CONNECT (asignación), ACK_BROKER (keep-alive)
    static void resolveBrokerMessages(int connectionId);

    // BALANCEO DE CARGA: Encuentra el servidor con menos clientes conectados
    // RETORNA: connectionId del servidor seleccionado, o -1 si no hay disponibles
    // ESTRATEGIA: Round-robin basado en carga (no todos los clientes van al mismo servidor)
    static int findServerWithLessClients();

    // DETECCIÓN DE FALLOS: Thread que monitorea constantemente la salud de los servidores
    // Corre en bucle infinito, cada X segundos verifica quién no envió keep-alive
    // Si un servidor no responde, se marca como "not alive" y no se asigna a clientes nuevos
    static void keepAliveMonitor();
};

