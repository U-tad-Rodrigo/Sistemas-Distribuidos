// EJEMPLOS DE IMPLEMENTACIÓN - Práctica 1
// Este archivo contiene ejemplos de código para guiarte en la implementación

/*
 * EJEMPLO 1: Protocolo de mensajes
 * Define los tipos de mensajes que se intercambiarán entre cliente y servidor
 */

// En clientManager.h ya tienes un enum msgTypes, amplíalo según necesites:
/*
typedef enum {
    MSG_LIST_FILES,          // Cliente solicita lista de archivos
    MSG_LIST_FILES_RESPONSE, // Servidor responde con lista
    MSG_READ_FILE,           // Cliente solicita leer archivo
    MSG_READ_FILE_RESPONSE,  // Servidor responde con contenido
    MSG_WRITE_FILE,          // Cliente solicita escribir archivo
    MSG_WRITE_FILE_RESPONSE, // Servidor confirma escritura
    MSG_CONSTRUCTOR,         // Cliente solicita crear instancia
    MSG_ACK                  // Mensaje de confirmación
} msgTypes;
*/

/*
 * EJEMPLO 2: Estructura básica del servidor
 */
/*
int main(int argc, char** argv) {
    int puerto = 8080;  // Puerto donde escuchará el servidor

    cout << "Iniciando servidor en puerto " << puerto << endl;
    int server_fd = initServer(puerto);

    cout << "Servidor esperando conexiones..." << endl;

    while (true) {
        // Esperar nueva conexión
        if (checkClient()) {
            int clientId = getLastClientID();
            cout << "Nuevo cliente conectado: " << clientId << endl;

            // Crear thread para atender al cliente
            thread clientThread(clientManager::atiendeCliente, clientId);
            clientThread.detach();
        }

        // Pequeña pausa para no consumir CPU
        this_thread::sleep_for(chrono::milliseconds(100));
    }

    return 0;
}
*/

/*
 * EJEMPLO 3: Implementación de atiendeCliente
 */
/*
void clientManager::atiendeCliente(int clientId) {
    cout << "Atendiendo cliente " << clientId << endl;

    // Crear instancia de FileManager para este cliente
    FileManager fm("FileManagerDir");
    instanciasFileManager[clientId] = fm;

    bool continuar = true;
    while (continuar) {
        try {
            // Recibir tipo de operación
            vector<int> tipoMsg;
            recvMSG(clientId, tipoMsg);

            int operacion = tipoMsg[0];

            switch (operacion) {
                case MSG_LIST_FILES: {
                    // Obtener lista de archivos
                    vector<string> files = instanciasFileManager[clientId].listFiles();

                    // Preparar respuesta
                    // 1. Enviar número de archivos
                    vector<int> numFiles = {(int)files.size()};
                    sendMSG(clientId, numFiles);

                    // 2. Enviar cada nombre de archivo
                    for (const string& filename : files) {
                        vector<char> nombreArchivo(filename.begin(), filename.end());
                        sendMSG(clientId, nombreArchivo);
                    }
                    break;
                }

                case MSG_READ_FILE: {
                    // Recibir nombre del archivo
                    vector<char> nombreBuf;
                    recvMSG(clientId, nombreBuf);
                    string filename(nombreBuf.begin(), nombreBuf.end());

                    // Leer archivo
                    vector<unsigned char> data;
                    instanciasFileManager[clientId].readFile(filename, data);

                    // Enviar datos
                    sendMSG(clientId, data);
                    break;
                }

                case MSG_WRITE_FILE: {
                    // Recibir nombre del archivo
                    vector<char> nombreBuf;
                    recvMSG(clientId, nombreBuf);
                    string filename(nombreBuf.begin(), nombreBuf.end());

                    // Recibir datos del archivo
                    vector<unsigned char> data;
                    recvMSG(clientId, data);

                    // Escribir archivo
                    instanciasFileManager[clientId].writeFile(filename, data);

                    // Enviar confirmación
                    vector<int> ack = {MSG_ACK};
                    sendMSG(clientId, ack);
                    break;
                }

                default:
                    continuar = false;
                    break;
            }
        } catch (...) {
            continuar = false;
        }
    }

    closeConnection(clientId);
    instanciasFileManager.erase(clientId);
}
*/

/*
 * EJEMPLO 4: Estructura básica del cliente
 */
/*
int main(int argc, char** argv) {
    string host = "127.0.0.1";  // localhost
    int puerto = 8080;

    cout << "Conectando al servidor..." << endl;
    connection_t conn = initClient(host, puerto);

    if (!conn.alive) {
        cout << "Error al conectar con el servidor" << endl;
        return 1;
    }

    int clientId = conn.serverId;
    cout << "Conectado al servidor" << endl;

    // Ahora implementar la lógica similar a main_fm.cpp
    // pero enviando peticiones al servidor en lugar de ejecutar localmente

    string command;
    do {
        cout << "Enter command:" << endl;
        cin >> command;

        if (command == "lls") {
            // Solicitar lista de archivos
            vector<int> msg = {MSG_LIST_FILES};
            sendMSG(clientId, msg);

            // Recibir número de archivos
            vector<int> numFiles;
            recvMSG(clientId, numFiles);

            cout << "Archivos remotos:" << endl;
            for (int i = 0; i < numFiles[0]; i++) {
                vector<char> nombreBuf;
                recvMSG(clientId, nombreBuf);
                string filename(nombreBuf.begin(), nombreBuf.end());
                cout << filename << endl;
            }
        }
        // ... otros comandos

    } while (command != "exit()");

    closeConnection(clientId);
    return 0;
}
*/

/*
 * NOTAS IMPORTANTES:
 *
 * 1. El protocolo debe ser consistente entre cliente y servidor
 * 2. Siempre verifica que las conexiones estén activas antes de enviar/recibir
 * 3. Usa try-catch para manejar errores de red
 * 4. Los vectores deben tener el tipo correcto (int, char, unsigned char)
 * 5. Recuerda convertir entre string y vector<char> cuando sea necesario
 * 6. Para enviar strings: vector<char> v(str.begin(), str.end())
 * 7. Para recibir strings: string str(vec.begin(), vec.end())
 */

