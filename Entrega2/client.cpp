/*
 * client.cpp - Cliente TCP interactivo
 *
 * Funcionalidades:
 * - Conecta a un servidor TCP (IP y puerto configurables)
 * - Envía mensajes del usuario al servidor
 * - Recibe y muestra respuestas y broadcasts del servidor
 * - Comandos especiales: "usuarios" (lista clientes), "exit" (desconectar)
 */

#include <iostream>
#include <string>
#include <thread>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

// Variable para controlar el hilo de recepción
bool conectado = true;

// Función que recibe mensajes del servidor en un hilo separado
// Esto permite recibir broadcasts mientras el usuario escribe
void recibirMensajes(int clientSocket) {
    char buffer[1024];

    while (conectado) {
        memset(buffer, 0, sizeof(buffer));

        int bytesRecibidos = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        if (bytesRecibidos <= 0) {
            if (conectado) {
                cerr << "\n[ERROR] Conexión con el servidor perdida" << endl;
                conectado = false;
            }
            break;
        }

        buffer[bytesRecibidos] = '\0';
        cout << buffer << flush;
    }
}

int main(int argc, char** argv) {
    // Configurar IP y puerto (defaults: 127.0.0.1:5000)
    string host = "127.0.0.1";
    int puerto = 5000;

    if (argc >= 2) {
        host = argv[1];
    }
    if (argc >= 3) {
        puerto = atoi(argv[2]);
        if (puerto <= 0 || puerto > 65535) {
            cerr << "Puerto inválido. Usando puerto por defecto: 5000" << endl;
            puerto = 5000;
        }
    }

    cout << "========================================" << endl;
    cout << "       CLIENTE TCP INTERACTIVO" << endl;
    cout << "========================================" << endl;
    cout << "Conectando a " << host << ":" << puerto << "..." << endl;

    // Crear socket del cliente
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        cerr << "Error al crear socket" << endl;
        return 1;
    }

    // Configurar dirección del servidor
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(puerto);

    // Convertir IP de texto a binario
    if (inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr) <= 0) {
        cerr << "Dirección IP inválida: " << host << endl;
        close(clientSocket);
        return 1;
    }

    // Conectar al servidor
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Error al conectar con el servidor" << endl;
        cerr << "Asegúrate de que el servidor esté ejecutándose en "
             << host << ":" << puerto << endl;
        close(clientSocket);
        return 1;
    }

    cout << "¡Conexión establecida!" << endl;
    cout << "========================================" << endl;
    cout << "Comandos disponibles:" << endl;
    cout << "  - Escribe un mensaje para enviarlo" << endl;
    cout << "  - 'usuarios' para ver clientes conectados" << endl;
    cout << "  - 'exit' para desconectar" << endl;
    cout << "========================================" << endl << endl;

    // Crear hilo para recibir mensajes del servidor
    thread hiloRecepcion(recibirMensajes, clientSocket);

    // Bucle principal: leer mensajes del usuario
    string mensaje;
    while (conectado) {
        cout << "> ";
        if (!getline(cin, mensaje)) {
            // EOF o error en entrada
            break;
        }

        // Ignorar líneas vacías
        if (mensaje.empty()) {
            continue;
        }

        // Enviar mensaje al servidor
        mensaje += "\n";  // Agregar salto de línea
        int byteEnviados = send(clientSocket, mensaje.c_str(), mensaje.length(), 0);

        if (byteEnviados <= 0) {
            cerr << "[ERROR] No se pudo enviar el mensaje" << endl;
            break;
        }

        // Si el usuario escribió "exit", prepararse para cerrar
        if (mensaje == "exit\n") {
            cout << "Cerrando conexión..." << endl;
            conectado = false;
            break;
        }
    }

    // Limpiar y cerrar
    conectado = false;
    close(clientSocket);

    // Esperar a que termine el hilo de recepción
    if (hiloRecepcion.joinable()) {
        hiloRecepcion.join();
    }

    cout << "Desconectado." << endl;

    return 0;
}