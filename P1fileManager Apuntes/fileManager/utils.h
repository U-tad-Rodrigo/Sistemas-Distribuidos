/*Rodrigo Fernández
* 05/11/2205
 */

#ifndef  _UTILS_H_
#define  _UTILS_H_

// ==================== INCLUDES DE SOCKETS Y COMUNICACIÓN ====================
#include <sys/types.h>
#include <sys/socket.h>      // API de sockets POSIX
#include <arpa/inet.h>        // Conversión de direcciones IP (inet_pton, etc.)

#include <netinet/in.h>       // Estructuras sockaddr_in
#include <unistd.h>           // read(), write(), close()
#include <string.h>           // memcpy()
#include <iostream>
#include <list>
#include <map>
#include <vector>
#include <thread>
#include <mutex>



#define DEBUG

#ifdef DEBUG

#define DEBUG_MSG(...)  printf(__VA_ARGS__);
#endif

// ==================== ESTRUCTURA: MENSAJE ====================
// CONCEPTO: Representa un mensaje almacenado temporalmente
// USO: Para comunicación asíncrona (no usado en este proyecto, pero disponible)
typedef struct msg_t
{
    int size;              // Tamaño del mensaje en bytes
    unsigned char* data;   // Puntero a los datos del mensaje
}msg_t;

// ==================== ESTRUCTURA: CONEXIÓN ====================
// CONCEPTO: Representa una conexión TCP activa (cliente o servidor)
// IMPORTANTE: Tanto clientes como servidores usan esta estructura
typedef struct connection_t{
    unsigned int id;           // ID único de la conexión (asignado por el servidor)
    unsigned int serverId;     // ID local del socket (asignado por el cliente)
    int socket;                // Descriptor de archivo del socket TCP
    std::list<msg_t*>* buffer; // Buffer de mensajes (para modo asíncrono)
    bool alive;                // Estado de la conexión (true = activa)
}connection_t;

// ==================== FUNCIONES DE INICIALIZACIÓN ====================

// SERVIDOR: Crear socket en modo escucha (listen)
// RETORNA: Descriptor del socket servidor
// OPERACIONES: socket() -> bind() -> listen() -> lanza thread para accept()
int initServer(int port);

// VERIFICAR: ¿Hay clientes esperando en la cola?
// RETORNA: true si hay conexiones pendientes
// USO: Bucle de espera activa en el main del servidor
bool checkClient();

// CLIENTE: Conectarse a un servidor remoto
// RETORNA: Estructura connection_t con info de la conexión
// OPERACIONES: socket() -> connect()
connection_t initClient(std::string host, int port);


// ==================== FUNCIONES DE COMUNICACIÓN (TEMPLATE) ====================
// IMPORTANTE: Estas son funciones template que funcionan con cualquier tipo de datos

// ENVIAR MENSAJE: Serializa y envía un vector de datos
// PROTOCOLO: Primero envía tamaño (int), luego los datos
// PARÁMETRO t: Tipo de datos (char, int, unsigned char, etc.)
template<typename t>
void sendMSG(int clientID, std::vector<t> &data);

// RECIBIR MENSAJE: Lee datos del socket y los almacena en el vector
// PROTOCOLO: Primero lee tamaño (int), luego lee los datos
// BLOQUEANTE: Espera hasta recibir todos los datos
template<typename t>
void recvMSG(int clientID, std::vector<t> &data);

// ==================== FUNCIONES DE GESTIÓN DE CONEXIONES ====================

// ESPERAR CONEXIONES: Bloquea hasta que un cliente se conecte (accept)
// RETORNA: Descriptor del nuevo socket de cliente
// NOTA: En este proyecto se usa waitForConnectionsAsync (en thread)
int waitForConnections(int sock_fd);

// CERRAR CONEXIÓN: Cierra socket y libera recursos
// OPERACIONES: close(socket) -> elimina del mapa de conexiones
void closeConnection(int clientID);

// OBTENER MENSAJE: Lee mensaje del buffer (modo asíncrono)
// NOTA: No usado en este proyecto (usamos comunicación síncrona)
template<typename t>
void getMSG(int clientID, std::vector<t> &data);

// ==================== FUNCIONES ASÍNCRONAS ====================
// NOTA: Estos métodos permiten comunicación no bloqueante

// VERIFICAR: ¿Hay mensajes pendientes en el buffer?
bool checkPendingMessages(int clientID);

// RECIBIR ASÍNCRONO: Lee mensajes en background (en un thread)
void recvMSGAsync(connection_t connection);

// ACEPTAR ASÍNCRONO: Espera conexiones en background (en un thread)
void waitForConnectionsAsync(int server_fd);

// ==================== FUNCIONES DE GESTIÓN DE CLIENTES ====================

// Obtener número total de clientes conectados
int getNumClients();

// Obtener ID de un cliente específico por índice
int getClientID(int numClient);

// Obtener ID del último cliente que se conectó
// IMPORTANTE: También lo elimina de la cola de espera
int getLastClientID();



// ==================== MAPA GLOBAL DE CONEXIONES ====================
// CONCEPTO: Almacena todas las conexiones activas (cliente o servidor)
// KEY: clientID (identificador único)
// VALUE: Estructura connection_t con información de la conexión
// IMPORTANTE: Es una variable global compartida entre todos los threads
extern std::map<unsigned int,connection_t> clientList;


// ==================== IMPLEMENTACIÓN DE RECVMSG (TEMPLATE) ====================
// CONCEPTO: Recibe mensaje completo desde el socket
// PROTOCOLO: 1) Lee 4 bytes (int) con el tamaño, 2) Lee los datos
template<typename t>
void recvMSG(int clientID, std::vector<t> &data){

    // Obtener info de la conexión desde el mapa global
    connection_t connection=clientList[clientID];

    int socket= connection.socket;

    // ==================== PASO 1: LEER TAMAÑO DEL MENSAJE ====================
    int bufferSize=0;
    int readData=read(socket, &bufferSize, sizeof(int));  // Leer 4 bytes
    DEBUG_MSG("DatosLeidos : %d\n",bufferSize);
	if(readData==0)
    {
       // DESCONEXIÓN: Si read() retorna 0, el socket se cerró
       printf("ERROR: recvMSG -- line : %d lost connection\n", __LINE__);
    }
	
    // ==================== PASO 2: LEER LOS DATOS ====================
    // CÁLCULO: Cuántos elementos de tipo t caben en bufferSize bytes
    int numElements=bufferSize/sizeof(t);
    data.resize(numElements);  // Reservar espacio

    int remaining=bufferSize;  // Bytes que faltan por leer
    int idxIn=0;

    // BUCLE: Leer hasta completar todo el mensaje
    // IMPORTANTE: read() puede retornar menos bytes de los solicitados
    while(remaining>0)
    {
        int bufferSizeBlock=read(socket, &(data.data()[bufferSize-remaining]),remaining);
        remaining-=bufferSizeBlock;        
    }
    
    // VALIDACIÓN: Verificar que se leyó todo correctamente
    if(remaining!=0)
    {
       printf("ERROR: recvMSG -- line : %d error data not matching: %d read, %d espected\n", __LINE__,remaining,bufferSize);
    }
}


// ==================== IMPLEMENTACIÓN DE SENDMSG (TEMPLATE) ====================
// CONCEPTO: Envía mensaje completo por el socket
// PROTOCOLO: 1) Envía tamaño (int), 2) Envía datos
template<typename t>
void sendMSG(int clientID, std::vector<t> &data){

    // CALCULAR: Tamaño total en bytes
    int dataLen=data.size()*sizeof(t);

    // Obtener info de la conexión
    connection_t connection=clientList[clientID];
    int socket= connection.socket;

    // ==================== PASO 1: ENVIAR TAMAÑO ====================
    // IMPORTANTE: El receptor necesita saber cuántos bytes leer
    write(socket,&dataLen,sizeof(int));

    // ==================== PASO 2: ENVIAR DATOS ====================
    write(socket,data.data(),dataLen);
}

// ==================== IMPLEMENTACIÓN DE GETMSG (ASÍNCRONO) ====================
// CONCEPTO: Obtiene mensaje del buffer (modo asíncrono)
// NOTA: No usado en este proyecto
template<typename t>
void getMSG(int clientID,std::vector<t> &data)
{
    if(!checkPendingMessages(clientID))
    {
	data.resize(0);  // No hay mensajes
    }
    else
    {
        // Extraer primer mensaje del buffer
        msg_t* msg=clientList[clientID].buffer->front();
        clientList[clientID].buffer->pop_front();

        // Copiar datos
        int numElem=msg->size/sizeof(t);
        data.resize(numElem);
        memcpy(data.data(),msg->data,msg->size);

        // Liberar memoria
	delete[] msg->data;
        delete[] msg;
    }
}



// ==================== FUNCIONES DE SERIALIZACIÓN (PACK/UNPACK) ====================
// CONCEPTO: Convertir datos de C++ a bytes para transmitir por red
// PATRÓN: Marshalling/Unmarshalling - serialización de estructuras

// PACK: Añadir un dato al final del buffer
// OPERACIÓN: Copia los bytes del dato al vector
template<typename T>
inline void pack(std::vector<unsigned char> &packet,T data){
	
	int size=packet.size();
	unsigned char *ptr=(unsigned char*)&data;  // Puntero a los bytes del dato
	packet.resize(size+sizeof(T));  // Ampliar el vector
	std::copy(ptr,ptr+sizeof(T),packet.begin()+size);  // Copiar bytes

}


// PACKV: Añadir un array de datos al buffer
// OPERACIÓN: Llama a pack() para cada elemento
template<typename T>
inline void packv(std::vector<unsigned char> &packet,T* data,int dataSize)
{
	for(int i=0;i<dataSize;i++)
			 	pack(packet,data[i]);
}



// UNPACK: Extraer un dato del inicio del buffer
// OPERACIÓN: Lee bytes, los convierte al tipo T, y los elimina del buffer
// IMPORTANTE: Modifica el buffer (elimina lo que lee)
template<typename T>
inline T unpack(std::vector<unsigned char> &packet){	
	T data;
	long int dataSize=sizeof(T);
	int packetSize=packet.size();

	// EXTRAER: Copiar bytes del inicio
	memcpy(&data,packet.data(),dataSize);

	// DESPLAZAR: Mover el resto del buffer hacia adelante
	memcpy(packet.data(),packet.data()+dataSize,packetSize-dataSize);

	// REDUCIR: Ajustar tamaño del buffer
	packet.resize(packetSize-dataSize);
	return data;
}



// UNPACKV: Extraer un array de datos del buffer
// OPERACIÓN: Lee múltiples elementos de tipo T
template<typename T>
inline void unpackv(std::vector<unsigned char> &packet,T* data,int dataSize)
{
	dataSize*=sizeof(T);  // Convertir número de elementos a bytes
	int packetSize=packet.size();

	// EXTRAER: Copiar bytes
	memcpy(data,packet.data(),dataSize);

	// DESPLAZAR: Mover el resto del buffer
	memcpy(packet.data(),packet.data()+dataSize,packetSize-dataSize);

	// REDUCIR: Ajustar tamaño
	packet.resize(packetSize-dataSize);
}

#endif

