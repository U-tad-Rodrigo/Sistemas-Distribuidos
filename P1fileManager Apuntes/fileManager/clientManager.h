/*Rodrigo Fernández
* 05/11/2205
 */
#pragma once

#include "utils.h"
#include <string>
#include "fileManager.h"

using namespace std;

// ==================== TIPOS DE MENSAJES CLIENTE-SERVIDOR ====================
// CONCEPTO: Protocolo de comunicación - define qué operaciones puede solicitar un cliente
// IMPORTANTE: Estos mensajes NO pasan por el broker, son directos cliente->servidor
// PATRÓN: RPC (Remote Procedure Call) - invocar funciones remotas como si fueran locales
typedef enum{
		constructorFilemanager,        // Crear instancia de FileManager (sin parámetros)
		constructorFilemanagerParams,  // Crear instancia de FileManager (con directorio)
		destructorFilemanager,         // Destruir instancia (cerrar sesión)
		listFilesF,                    // Listar archivos del directorio
		readFileF,                     // Leer contenido de un archivo
		writeFileF,                    // Escribir/crear un archivo
		ack                            // Confirmación (acknowledgment)
}msgTypes;

// ==================== CLASE CLIENT MANAGER ====================
// RESPONSABILIDAD: Gestionar la comunicación entre clientes y el servidor
// IMPORTANTE: Esta clase se usa en AMBOS lados (cliente y servidor)
class clientManager{
public:
	// ==================== LADO CLIENTE ====================
	// CONCEPTO: Mapeo de objetos FileManager locales a sus conexiones remotas
	// KEY: Puntero al objeto FileManager local
	// VALUE: connectionId del servidor donde está la instancia real
	// USO: Cuando el cliente llama fm.readFile(), necesitamos saber a qué servidor enviar
	static inline map<FileManager*, int> connectionIds;

	// ==================== LADO SERVIDOR ====================
	// CONCEPTO: Cada cliente tiene su propia instancia de FileManager en el servidor
	// KEY: clientId (identificador único del cliente conectado)
	// VALUE: Instancia real de FileManager que procesa las operaciones
	// IMPORTANTE: Mantiene estado por cliente (sesión)
	static inline map<int,FileManager> instanciasFileManager;

	// ==================== MÉTODO PRINCIPAL (LADO SERVIDOR) ====================
	// FUNCIÓN: Thread que procesa todas las peticiones de un cliente específico
	// PARÁMETRO: clientId = identificador único del cliente
	// FLUJO: Bucle que recibe mensaje -> procesa -> responde -> repite hasta logout
	// IMPORTANTE: Corre en un thread separado por cada cliente conectado
	static void resolveClientMessages(int clientId);
	
};