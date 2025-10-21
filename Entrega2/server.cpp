/*
 * server.cpp - Servidor TCP multi-cliente con broadcast
 *
 * Funcionalidades:
 * - Acepta múltiples clientes simultáneos (un hilo por cliente)
 * - Broadcast: reenvía mensajes a todos los clientes excepto al emisor
 * - Comando "usuarios": lista los IDs de clientes conectados
 * - Comando "exit": desconecta al cliente que lo envía
 * - Puerto configurable por argumento (default: 5000)
 */

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <algorithm>

using namespace std;

// Variables globales para gestión de clientes
struct ClienteInfo {
    int socket;
    int id;
};

vector<ClienteInfo> clientesConectados;  // Lista de clientes activos
mutex clientesMutex;                     // Protege acceso a clientesConectados
atomic<int> contadorClientes(0);         // Contador de IDs de clientes
mutex coutMutex;                         // Protege salida a consola

// Función para eliminar saltos de línea al final de una cadena
string trim(const string& str) {
    size_t end = str.find_last_not_of("\r\n");
    return (end == string::npos) ? "" : str.substr(0, end + 1);
}

// Función para agregar un cliente a la lista global
void agregarCliente(int socket, int id) {
    lock_guard<mutex> lock(clientesMutex);
    clientesConectados.push_back({socket, id});
}

// Función para eliminar un cliente de la lista global
void eliminarCliente(int id) {
    lock_guard<mutex> lock(clientesMutex);
    clientesConectados.erase(
        remove_if(clientesConectados.begin(), clientesConectados.end(),
                  [id](const ClienteInfo& c) { return c.id == id; }),
        clientesConectados.end()
    );
}

// Función para hacer broadcast de un mensaje a todos excepto al emisor
void broadcast(const string& mensaje, int idEmisor) {
    lock_guard<mutex> lock(clientesMutex);
    for (const auto& cliente : clientesConectados) {
        if (cliente.id != idEmisor) {
            send(cliente.socket, mensaje.c_str(), mensaje.length(), 0);
        }
    }
}

// Función para obtener la lista de IDs conectados
string obtenerListaUsuarios() {
    lock_guard<mutex> lock(clientesMutex);
    if (clientesConectados.empty()) {
        return "conectados: ninguno";
    }

    string lista = "conectados: ";
    for (size_t i = 0; i < clientesConectados.size(); ++i) {
        lista += to_string(clientesConectados[i].id);
        if (i < clientesConectados.size() - 1) {
            lista += ",";
        }
    }
    return lista;
}

// Función que maneja la comunicación con un cliente específico
// Se ejecuta en un hilo separado
void manejarCliente(int clientSocket, int clientId) {
    char buffer[1024];

    {
        lock_guard<mutex> lock(coutMutex);
        cout << "[SERVIDOR] Cliente " << clientId << " conectado (socket: "
             << clientSocket << ")" << endl;
    }

    // Agregar cliente a la lista global
    agregarCliente(clientSocket, clientId);

    // Notificar a otros clientes
    string mensajeConexion = "cliente " + to_string(clientId) + " se ha conectado\n";
    broadcast(mensajeConexion, clientId);

    // Bucle principal: recibir y procesar mensajes
    while (true) {
        memset(buffer, 0, sizeof(buffer));

        int bytesRecibidos = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        if (bytesRecibidos <= 0) {
            // Cliente desconectado o error
            lock_guard<mutex> lock(coutMutex);
            cout << "[SERVIDOR] Cliente " << clientId << " desconectado" << endl;
            break;
        }

        buffer[bytesRecibidos] = '\0';
        string mensaje = trim(string(buffer));

        // Mostrar mensaje en consola del servidor
        {
            lock_guard<mutex> lock(coutMutex);
            cout << "cliente " << clientId << ": " << mensaje << endl;
        }

        // Procesar comandos especiales
        if (mensaje == "exit") {
            // Cliente solicita desconexión
            string respuesta = "Servidor: desconectando...\n";
            send(clientSocket, respuesta.c_str(), respuesta.length(), 0);
            break;
        }
        else if (mensaje == "usuarios") {
            // Listar usuarios conectados
            string listaUsuarios = obtenerListaUsuarios() + "\n";
            send(clientSocket, listaUsuarios.c_str(), listaUsuarios.length(), 0);
        }
        else {
            // Mensaje normal: hacer broadcast a otros clientes
            string mensajeBroadcast = "cliente " + to_string(clientId) + ": " + mensaje + "\n";
            broadcast(mensajeBroadcast, clientId);

            // Enviar ACK al emisor
            string ack = "Servidor: mensaje recibido correctamente.\n";
            send(clientSocket, ack.c_str(), ack.length(), 0);
        }
    }

    // Limpiar y cerrar
    eliminarCliente(clientId);
    close(clientSocket);

    // Notificar desconexión a otros clientes
    string mensajeDesconexion = "cliente " + to_string(clientId) + " se ha desconectado\n";
    broadcast(mensajeDesconexion, clientId);
}

// Función principal del servidor
int main(int argc, char** argv) {
    // Configurar puerto (default: 5000)
    int puerto = 5000;
    if (argc >= 2) {
        puerto = atoi(argv[1]);
        if (puerto <= 0 || puerto > 65535) {
            cerr << "Puerto inválido. Usando puerto por defecto: 5000" << endl;
            puerto = 5000;
        }
    }

    cout << "============================================" << endl;
    cout << "  SERVIDOR TCP MULTI-CLIENTE CON BROADCAST" << endl;
    cout << "============================================" << endl;
    cout << "Puerto: " << puerto << endl;
    cout << "============================================" << endl;

    // Crear socket del servidor
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        cerr << "Error al crear socket" << endl;
        return 1;
    }

    // Configurar reutilización de dirección
    int opcion = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opcion, sizeof(opcion)) < 0) {
        cerr << "Error en setsockopt" << endl;
        close(serverSocket);
        return 1;
    }

    // Configurar dirección del servidor
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(puerto);

    // Vincular socket a la dirección
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Error al hacer bind en puerto " << puerto << endl;
        cerr << "¿El puerto ya está en uso?" << endl;
        close(serverSocket);
        return 1;
    }

    // Escuchar conexiones entrantes
    if (listen(serverSocket, 10) < 0) {
        cerr << "Error al hacer listen" << endl;
        close(serverSocket);
        return 1;
    }

    cout << "[SERVIDOR] Escuchando en puerto " << puerto << "..." << endl;
    cout << "[SERVIDOR] Esperando conexiones de clientes..." << endl;
    cout << "============================================" << endl << endl;

    // Bucle principal: aceptar conexiones
    while (true) {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);

        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);

        if (clientSocket < 0) {
            cerr << "[ERROR] Fallo al aceptar conexión" << endl;
            continue;
        }

        // Asignar ID único al cliente
        int clientId = ++contadorClientes;

        // Crear hilo para manejar este cliente
        thread hiloCliente(manejarCliente, clientSocket, clientId);
        hiloCliente.detach();  // Independizar el hilo
    }

    close(serverSocket);
    return 0;
}