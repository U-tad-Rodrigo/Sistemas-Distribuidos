/*Rodrigo Fernández
* 05/11/2205
 */

#include "utils.h"
#include <map>
#include <thread>
#include <mutex>

// ==================== VARIABLES GLOBALES ====================
// IMPORTANTE: Compartidas entre todos los threads

// MAPA: Almacena todas las conexiones activas (cliente o servidor)
std::map<unsigned int,connection_t> clientList;

// CONTADOR: Asigna IDs únicos a cada conexión
unsigned int contador=0;

// FLAG: Para terminar threads (no usado en este proyecto)
bool salir=false;

// THREAD: Para aceptar conexiones en background
std::thread* waitForConnectionsThread;

// CONTROL: Número de clientes antes del último getLastClientID()
int lastClientSize=0;

// COLA: Clientes que están esperando ser procesados
std::list<unsigned int> waitingClients;

// MUTEX: Protege el contador de accesos concurrentes
std::mutex contador_mutex;

// ==================== FUNCIÓN: INICIALIZAR SERVIDOR ====================
// CONCEPTO: Crear socket TCP en modo escucha (servidor)
// RETORNA: Descriptor del socket servidor
// FASES: 1) Crear socket, 2) Configurar, 3) Bind, 4) Listen, 5) Lanzar thread
int initServer(int port)
{
    // ==================== PASO 1: CREAR SOCKET ====================
    // AF_INET = IPv4, SOCK_STREAM = TCP (confiable, orientado a conexión)
    int sock_fd;
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0)
    {
        printf("Error creating socket\n");
    }

    // ==================== PASO 2: CONFIGURAR DIRECCIÓN ====================
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;           // IPv4
    serv_addr.sin_addr.s_addr = INADDR_ANY;   // Escuchar en todas las interfaces
    serv_addr.sin_port = htons(port);         // Puerto (convertido a network byte order)

    // ==================== OPCIONES DE SOCKET ====================
    // SO_REUSEPORT | SO_REUSEADDR = Permitir reusar puerto inmediatamente
    // IMPORTANTE: Sin esto, el SO mantiene el puerto "ocupado" tras cerrar
    int option = 1;
    setsockopt(sock_fd,SOL_SOCKET,
               (SO_REUSEPORT | SO_REUSEADDR),
               &option,sizeof(option));

    // ==================== PASO 3: BIND ====================
    // OPERACIÓN: Asociar el socket al puerto especificado
    // EFECTO: El SO reserva el puerto para este proceso
    if (bind(sock_fd,(struct sockaddr * ) &serv_addr,
             sizeof(serv_addr)) < 0)
        printf("ERROR on binding");

    // ==================== PASO 4: LISTEN ====================
    // OPERACIÓN: Poner el socket en modo escucha
    // PARÁMETRO 5 = backlog (máximo de conexiones pendientes en cola)
    listen(sock_fd,5);

    // ==================== PASO 5: THREAD ASÍNCRONO ====================
    // CONCEPTO: Aceptar conexiones en background (no bloquear el main)
    // VENTAJA: El main puede hacer otras cosas mientras se aceptan clientes
    waitForConnectionsThread=new std::thread(waitForConnectionsAsync,sock_fd);
    return sock_fd;
}



// ==================== FUNCIÓN: INICIALIZAR CLIENTE ====================
// CONCEPTO: Conectarse a un servidor remoto
// RETORNA: Estructura connection_t con info de la conexión
// FASES: 1) Crear socket, 2) Configurar dirección, 3) Conectar, 4) Registrar
connection_t initClient(std::string host, int port)
{
    // ==================== PASO 1: CREAR SOCKET ====================
    int sock_out = 0;
    struct sockaddr_in serv_addr;
    connection_t connection;

    if ((sock_out = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        connection.socket=-1;
        connection.alive=false;
        return connection;
    }

    // ==================== PASO 2: CONFIGURAR DIRECCIÓN DEL SERVIDOR ====================
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);  // Puerto destino

    // CONVERTIR: IP de string a formato binario
    // EJEMPLO: "127.0.0.1" -> bytes [127, 0, 0, 1]
    if(inet_pton(AF_INET, host.c_str(), &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        connection.socket=-1;
        connection.alive=false;
        return connection;
    }

    // ==================== PASO 3: CONECTAR ====================
    // OPERACIÓN: Establecer conexión TCP con el servidor
    // IMPORTANTE: Esto hace el handshake TCP (SYN, SYN-ACK, ACK)
    if (connect(sock_out, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        connection.socket=-1;
        connection.alive=false;
        return connection;
    }
    
    unsigned int localID=-1;

    // ==================== PASO 4: REGISTRAR CONEXIÓN ====================
    connection.id=localID;
    connection.socket=sock_out;
    connection.buffer=new std::list<msg_t*>();

    // SECCIÓN CRÍTICA: Asignar ID único y guardar en el mapa
    contador_mutex.lock();
      connection.serverId=contador;
      clientList[contador]=connection;  // Registrar en mapa global
      contador++;  // Incrementar para el siguiente
    contador_mutex.unlock();

    return connection;
}


// ==================== FUNCIÓN: THREAD DE ACEPTACIÓN ====================
// CONCEPTO: Bucle infinito que acepta conexiones entrantes
// CONTEXTO: Corre en un thread separado (lanzado por initServer)
void waitForConnectionsAsync(int server_fd)
{
    while(!salir)
    {
        // BLOQUEANTE: Espera hasta que llegue una conexión
        int newSocket=waitForConnections(server_fd);
    }
}

// ==================== FUNCIÓN: ACEPTAR UNA CONEXIÓN ====================
// CONCEPTO: accept() bloquea hasta que un cliente se conecte
// RETORNA: Descriptor del nuevo socket de cliente
int waitForConnections(int sock_fd){
    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);

    // ==================== ACCEPT ====================
    // BLOQUEANTE: Espera hasta que llegue una conexión
    // EFECTO: Crea un NUEVO socket para comunicarse con este cliente
    // IMPORTANTE: sock_fd sigue escuchando, newsock_fd es para este cliente
    int newsock_fd = accept(sock_fd,
                            (struct sockaddr * ) &cli_addr,
                            &clilen);

    // ==================== REGISTRAR CLIENTE ====================
    connection_t client;

    // SECCIÓN CRÍTICA: Asignar ID único
    contador_mutex.lock();
     client.id=contador;
     contador++;
    contador_mutex.unlock();

    client.alive=true;
    client.socket=newsock_fd;
    client.buffer=new std::list<msg_t*>();
    clientList[client.id]=client;  // Guardar en mapa global

    // AGREGAR A COLA DE ESPERA: Para que el main lo procese
    waitingClients.push_back(client.id);

    return newsock_fd;
}

// ==================== FUNCIÓN: CERRAR CONEXIÓN ====================
// CONCEPTO: Liberar recursos y cerrar socket
// IMPORTANTE: Limpia memoria y elimina del mapa
void closeConnection(int clientID){
    connection_t connection=clientList[clientID];

    // CERRAR SOCKET: Termina la conexión TCP
    close(connection.socket);
    connection.alive=false;

    // LIMPIEZA: Verificar si hay mensajes sin leer (modo asíncrono)
    if(checkPendingMessages(clientID))
    {
      printf("ERROR: unread messages from %d\n",connection.id );

      // LIBERAR: Memoria de mensajes pendientes
      for(std::list<msg_t*>::iterator t=connection.buffer->begin();
          t!=connection.buffer->end();t++)
      {
          msg_t* msg=*t;
          delete[] msg->data;
          delete[] msg;
      }
      delete connection.buffer;

    }

    // ELIMINAR: Quitar del mapa global
    clientList.erase(clientID);
}


// ==================== FUNCIONES ASÍNCRONAS ====================
// NOTA: Estas funciones permiten comunicación no bloqueante
// No se usan en este proyecto (usamos comunicación síncrona)

// RECIBIR EN BACKGROUND: Lee mensajes continuamente en un thread
void recvMSGAsync(connection_t connection){

    while(connection.alive){
        msg_t* msg=new msg_t[1];
        std::vector<unsigned char> data;
        recvMSG<unsigned char>(connection.socket, data);
        msg->data=new unsigned char[data.size()];
        memcpy(msg->data,data.data(),data.size());
        msg->size=data.size();
        connection.buffer->push_back(msg);
    }
}

// VERIFICAR: ¿Hay mensajes en el buffer?
bool checkPendingMessages(int clientID)
{
    return clientList[clientID].buffer->size()>0 ;
}




// ==================== FUNCIONES DE GESTIÓN DE CLIENTES ====================

// VERIFICAR: ¿Hay clientes esperando ser procesados?
bool checkClient()
{
    return waitingClients.size()>0;

}

// OBTENER: Número total de conexiones activas
int getNumClients()
{
    return clientList.size();
}

// OBTENER: ID de un cliente por índice
int getClientID(int numClient)
{
    return clientList[numClient].id;

}

// OBTENER: ID del último cliente que se conectó
// IMPORTANTE: También lo elimina de la cola de espera
// USO: En el bucle principal del servidor para procesar clientes
int getLastClientID()
{
    int id=waitingClients.back();   // Obtener último
    waitingClients.pop_back();      // Eliminar de la cola
    return id;
}
