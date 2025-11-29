/*Rodrigo Fernández
* 05/11/2205
 */

#include "fileManager.h"
#include "utils.h"
#include "clientManager.h"
#include "brokerManager.h"

#define BROKER_IP "127.0.0.1"
#define BROKER_PORT 1066

// ==================== VARIABLES ESTÁTICAS (COMPARTIDAS POR TODAS LAS INSTANCIAS) ====================
// CONCEPTO: Service Discovery - el cliente necesita saber dónde conectarse
// IMPORTANTE: Se obtiene una vez del broker y se reutiliza para todas las instancias
// OPTIMIZACIÓN: No consultar al broker cada vez que se crea un FileManager
static string assignedServerIp = "";
static int assignedServerPort = 0;

// ==================== FUNCIÓN: OBTENER SERVIDOR DEL BROKER ====================
// CONCEPTO: Service Discovery - preguntar al broker por un servidor disponible
// FLUJO: Cliente -> Broker: "Dame un servidor" -> Broker: "Usa este IP:Puerto"
// IMPORTANTE: Solo se ejecuta una vez (static variables), las demás instancias reutilizan
void obtenerServidorDelBroker() {
    // OPTIMIZACIÓN: Si ya tenemos servidor asignado, no volver a consultar
    if (!assignedServerIp.empty()) {
        return;
    }

    cout << "[CLIENT] Consultando broker para obtener servidor..." << endl;

    // CONECTAR AL BROKER: Abrir conexión TCP temporal
    connection_t brokerConn = initClient(BROKER_IP, BROKER_PORT);

    // ==================== ENVIAR SOLICITUD ====================
    vector<unsigned char> buffer;
    pack(buffer, CLIENT_CONNECT);  // Tipo de mensaje: solicitud de servidor
    sendMSG(brokerConn.serverId, buffer);

    // ==================== RECIBIR RESPUESTA ====================
    recvMSG(brokerConn.serverId, buffer);
    brokerMsgTypes response = unpack<brokerMsgTypes>(buffer);  // Tipo de respuesta

    if (response == ACK_BROKER) {
        // DESEMPAQUETAR INFORMACIÓN DEL SERVIDOR ASIGNADO
        int ipSize = unpack<int>(buffer);  // Tamaño de la IP

        if (ipSize > 0) {
            // SERVIDOR DISPONIBLE: Extraer IP y puerto
            assignedServerIp.resize(ipSize);
            unpackv(buffer, (char*)assignedServerIp.data(), ipSize);
            assignedServerPort = unpack<int>(buffer);

            cout << "[CLIENT] Broker asignó servidor: " << assignedServerIp
                 << ":" << assignedServerPort << endl;
        } else {
            // ERROR: No hay servidores activos
            cout << "[CLIENT] ERROR: No hay servidores disponibles" << endl;
        }
    }

    // CERRAR: Conexión temporal con el broker
    closeConnection(brokerConn.serverId);
}

// ==================== CONSTRUCTOR SIN PARÁMETROS ====================
// CONCEPTO: Este constructor crea un FileManager remoto (RPC)
// FLUJO: 1) Obtener servidor del broker, 2) Conectarse al servidor, 3) Pedir crear instancia
/**
* @brief FileManager::FileManager Constructor without parameters of the FileManager class (empty). 
*
*/
FileManager::FileManager() {
	// ==================== PASO 1: SERVICE DISCOVERY ====================
	// Obtener IP:Puerto del servidor (si aún no tenemos)
	obtenerServidorDelBroker();

	// VALIDACIÓN: Verificar que el broker nos dio un servidor
	if (assignedServerIp.empty()) {
		cout << "ERROR: No se pudo obtener servidor del broker" << endl;
		return;
	}

	// ==================== PASO 2: CONECTAR AL SERVIDOR ====================
	// IMPORTANTE: Esta conexión es PERSISTENTE (no como con el broker)
	// Mantiene el socket abierto para futuras operaciones (listFiles, readFile, etc.)
	vector<unsigned char> buffer;
	int serverId = initClient(assignedServerIp, assignedServerPort).serverId;

	// REGISTRAR CONEXIÓN: Mapear este objeto a su conexión remota
	// CRUCIAL: Cuando llamemos this->readFile(), necesitamos saber a qué servidor enviar
	clientManager::connectionIds[this] = serverId;

	// ==================== PASO 3: INVOCAR CONSTRUCTOR REMOTO ====================
	// PROTOCOLO RPC: Enviar mensaje pidiendo crear FileManager en el servidor
	pack(buffer, constructorFilemanager);
	sendMSG(serverId, buffer);

	// ESPERAR CONFIRMACIÓN: Comunicación síncrona
	recvMSG(serverId, buffer);
	if (unpack<msgTypes>(buffer) != ack) {
		cout << "ERROR: FileManager::FileManager No ack received from server\n";
	}
	// NOTA: La conexión queda ABIERTA para futuras operaciones
}

// ==================== DESTRUCTOR ====================
// CONCEPTO: Liberar recursos remotos y cerrar conexión
// FLUJO: 1) Enviar mensaje de destrucción, 2) Esperar ACK, 3) Cerrar socket
/**
* @brief FileManager::FileManager Destructor without parameters of the FileManager class. 
*
*/
FileManager::~FileManager(){
	// OBTENER CONEXIÓN: Buscar a qué servidor está conectado este objeto
	int serverID = clientManager::connectionIds[this];

	// ENVIAR DESTRUCCIÓN REMOTA
	vector<unsigned char> buffer;
	pack(buffer, destructorFilemanager);
	sendMSG(serverID, buffer);

	// ESPERAR CONFIRMACIÓN
	recvMSG(serverID, buffer);
	if (unpack<msgTypes>(buffer)!= ack) {
		cout << "Error in destructor file manager" << endl;
	}
	// NOTA: El servidor cerrará la conexión tras enviar el ACK
}

// ==================== CONSTRUCTOR CON DIRECTORIO ====================
// CONCEPTO: Igual que el constructor sin parámetros, pero pasa un path al servidor
/**
* @brief FileManager::FileManager Constructor of the FileManager class. It receives by parameters the directory
* that this class will use to index, store and read files. It is recommended to use a full path to the directory,
* from the root of the file system.
*
* @param path Path to the directory you want to use.
*/
FileManager::FileManager(string path) {
	// ==================== SERVICE DISCOVERY ====================
	obtenerServidorDelBroker();

	if (assignedServerIp.empty()) {
		cout << "ERROR: No se pudo obtener servidor del broker" << endl;
		return;
	}

	// ==================== CONECTAR AL SERVIDOR ====================
	int serverId = initClient(assignedServerIp, assignedServerPort).serverId;

	// IMPORTANTE: Guardar la conexión en el mapa
	// Sin esto, las llamadas posteriores no sabrían a qué servidor enviar
	clientManager::connectionIds[this] = serverId;

	// ==================== INVOCAR CONSTRUCTOR CON PARÁMETROS ====================
	vector<unsigned char> buffer;
	pack(buffer, constructorFilemanagerParams);  // Tipo de mensaje

	// EMPAQUETAR PARÁMETRO: El path como string
	pack(buffer, (int)path.size());
	packv(buffer, (unsigned char *)path.data(), (int)path.size());

	sendMSG(serverId, buffer);

	// ESPERAR CONFIRMACIÓN
	recvMSG(serverId, buffer);
	if (unpack<msgTypes>(buffer) != ack) {
		cout << "ERROR: FileManager::FileManager No ack received from server\n";
	}
}

// ==================== LISTAR ARCHIVOS (RPC) ====================
// CONCEPTO: Llamada remota - el servidor ejecuta la operación y devuelve resultado
// PROTOCOLO: Request (vacío) -> Response (lista de strings)
/**
 * @brief FileManager::listFiles Used to access the list of files stored in the path
 * that was used in the class constructor. Only lists files, directories are ignored.
 *
 */
vector<string> FileManager::listFiles(){
	vector<unsigned char> buffer;

	// OBTENER CONEXIÓN: ¿A qué servidor pertenece este objeto?
	int serverId = clientManager::connectionIds[this];

	// ENVIAR PETICIÓN: Solicitar lista de archivos
	pack(buffer, listFilesF);
	sendMSG(serverId, buffer);

	// ESPERAR RESPUESTA
	recvMSG(serverId, buffer);

	// ==================== DESEMPAQUETAR RESPUESTA ====================
	// PROTOCOLO: Número de strings + (tamaño + datos) por cada string
	vector<string> resultado;
	resultado.resize(unpack<int>(buffer));  // Cantidad de archivos
	for (auto& fileName : resultado) {
		fileName.resize(unpack<int>(buffer));  // Tamaño del nombre
		unpackv(buffer, (char*)fileName.data(), (int)fileName.size());  // Caracteres
	}
	return resultado;
}

// ==================== LEER ARCHIVO (RPC) ====================
// CONCEPTO: Enviar nombre del archivo, recibir su contenido en bytes
/**
 * @brief FileManager::readFile Given the name of a file stored in the directory used in the constructor,
 * the variable "data" will be filled with the contents of the file
 *
 * @param fileName Name of the file to read
 * @param data File data
 */
void FileManager::readFile(string fileName, vector<unsigned char> &data){
	int serverId = clientManager::connectionIds[this];

	// EMPAQUETAR PETICIÓN: Tipo + nombre del archivo
	vector<unsigned char> buffer;
	pack(buffer, readFileF);
	pack(buffer, (int)fileName.size());
	packv(buffer, (unsigned char *)fileName.data(), (int)fileName.size());

	sendMSG(serverId, buffer);

	// RECIBIR RESPUESTA: Contenido del archivo
	recvMSG(serverId, buffer);

	// DESEMPAQUETAR: Extraer bytes del archivo
	data.resize(unpack<int>(buffer));  // Tamaño del archivo
	unpackv(buffer, data.data(), (int)data.size());  // Datos binarios
}

// ==================== ESCRIBIR ARCHIVO (RPC) ====================
// CONCEPTO: Enviar nombre + contenido, el servidor lo guarda en disco
/**
 * @brief FileManager::writeFile Given a new name of a file to be stored in the directory used in the constructor,
 * the contents of the data array stored in "data" will be written. It will overwrite a file in the directory if it has the same name.
 *
 * @param fileName Name of the file to write.
 * @param data Data of the file.
 */
void FileManager::writeFile(string fileName, vector<unsigned char> &data){
	int serverId = clientManager::connectionIds[this];

	// EMPAQUETAR PETICIÓN: Tipo + nombre + contenido
	vector<unsigned char> buffer;
	pack(buffer, writeFileF);
	pack(buffer, (int)fileName.size());
	packv(buffer, (unsigned char *)fileName.data(), (int)fileName.size());

	pack(buffer, (int)data.size());  // Tamaño del contenido
	packv(buffer, data.data(), (int)data.size());  // Contenido binario

	sendMSG(serverId, buffer);

	// ESPERAR CONFIRMACIÓN
	recvMSG(serverId, buffer);
	if (unpack<msgTypes>(buffer) != ack) {
		cout << "ERROR: FileManager::writeFile No ack received from server\n";
	}
}