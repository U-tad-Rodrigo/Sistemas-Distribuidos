/*Rodrigo Fernández
 * 05/11/2205
 */

#include "clientManager.h"

// ==================== FUNCIÓN PRINCIPAL: RESOLVER MENSAJES DEL CLIENTE ====================
// CONCEPTO: Esta función implementa el SERVIDOR (server-side) del protocolo RPC
// PATRÓN: Request-Response - recibe petición, procesa, envía respuesta
// IMPORTANTE: Corre en un thread dedicado para cada cliente conectado
// PARÁMETRO: clientId = identificador único de este cliente
void clientManager::resolveClientMessages(int clientId) {
	string userName="";
	vector<unsigned char> buffer;
	bool logOut=false;

	// ==================== BUCLE PRINCIPAL: PROCESAR PETICIONES ====================
	// FLUJO: Mientras el cliente no cierre sesión (destructor), procesar mensajes
	// IMPORTANTE: Este bucle mantiene la conexión persistente entre cliente y servidor
	do {
		// ==================== PASO 1: RECIBIR MENSAJE ====================
		// recvMSG() bloquea hasta recibir datos completos del cliente
		recvMSG(clientId,buffer);

		// ==================== PASO 2: IDENTIFICAR TIPO DE PETICIÓN ====================
		msgTypes type=unpack<msgTypes>(buffer);

		// ==================== SWITCH: DESPACHAR SEGÚN TIPO DE MENSAJE ====================
		// PATRÓN: Command Pattern - cada case ejecuta una operación específica
		switch(type) {
			// ==================== CONSTRUCTOR SIN PARÁMETROS ====================
			// FLUJO: Cliente crea FileManager() -> Servidor crea instancia local
			case constructorFilemanager: {
				FileManager fm;  // Crear instancia en memoria del servidor
				instanciasFileManager[clientId]=fm;  // Asociar con este cliente

				// RESPONDER CON ACK: Confirmar que se creó correctamente
				buffer.clear();
				pack(buffer,ack);
				sendMSG(clientId,buffer);
			}break;

			// ==================== CONSTRUCTOR CON DIRECTORIO ====================
			// FLUJO: Cliente crea FileManager("/ruta") -> Servidor crea con ese path
			case constructorFilemanagerParams: {
				FileManager fm;
				string path;

				// DESEMPAQUETAR: Extraer parámetro (path) del mensaje
				// PROTOCOLO: Primero tamaño, luego datos (para strings)
				path.resize(unpack<int>(buffer));  // Leer tamaño
				unpackv(buffer,(char*)path.data(),(int)path.size());  // Leer caracteres

				// CREAR INSTANCIA: Con el directorio especificado
				fm=FileManager(path);
				instanciasFileManager[clientId]=fm;  // Guardar para este cliente

				// CONFIRMAR: Enviar ACK
				buffer.clear();
				pack(buffer,ack);
				sendMSG(clientId,buffer);
			}break;

			// ==================== DESTRUCTOR (LOGOUT) ====================
			// FLUJO: Cliente destruye FileManager -> Servidor libera recursos
			// IMPORTANTE: Esto indica que el cliente terminó su sesión
			case destructorFilemanager:
			{
				// LIMPIAR: Eliminar instancia del mapa (libera memoria)
				instanciasFileManager.erase(clientId);

				// CONFIRMAR: Enviar ACK
				buffer.clear();
				pack(buffer,ack);
				sendMSG(clientId,buffer);

				// MARCAR LOGOUT: Salir del bucle y cerrar conexión
				logOut=true;
			}break;

			// ==================== LISTAR ARCHIVOS ====================
			// FLUJO: Cliente llama listFiles() -> Servidor devuelve lista
			case listFilesF:
			{
				// INVOCAR: Llamar al método real de FileManager
				// IMPORTANTE: Usamos la instancia asociada a este cliente
				vector<string> resultado=instanciasFileManager[clientId].listFiles();

				// ==================== EMPAQUETAR RESULTADO ====================
				// PROTOCOLO: Para un vector de strings:
				// 1) Número de elementos
				// 2) Para cada elemento: tamaño + datos
				buffer.clear();
				pack(buffer,(int)resultado.size());  // Cantidad de archivos
				for(auto &fileName: resultado){
					pack(buffer,(int)fileName.size());  // Tamaño del nombre
					packv(buffer,(char*)fileName.data(),(int)fileName.size());  // Nombre
				}

				// ENVIAR: Respuesta al cliente
				sendMSG(clientId,buffer);
			}break;

			// ==================== LEER ARCHIVO ====================
			// FLUJO: Cliente llama readFile(name) -> Servidor lee y devuelve contenido
			case readFileF: {
				string file;
				vector<unsigned char> data;

				// DESEMPAQUETAR: Extraer nombre del archivo
				file.resize(unpack<int>(buffer));
				unpackv(buffer,(char*)file.data(),(int)file.size());

				// INVOCAR: Leer archivo desde el sistema de archivos
				instanciasFileManager[clientId].readFile(file, data);

				// EMPAQUETAR RESPUESTA: Tamaño + contenido del archivo
				buffer.clear();
				pack(buffer,(int)data.size());  // Tamaño en bytes
				packv(buffer,(char*)data.data(),(int)data.size());  // Datos binarios

				// ENVIAR: Contenido al cliente
				sendMSG(clientId,buffer);
			}break;

			// ==================== ESCRIBIR ARCHIVO ====================
			// FLUJO: Cliente llama writeFile(name, data) -> Servidor escribe en disco
			case writeFileF: {
				string file;
				// DESEMPAQUETAR NOMBRE
				file.resize(unpack<int>(buffer));
				unpackv(buffer,(char*)file.data(),(int)file.size());

				// DESEMPAQUETAR DATOS
				vector<unsigned char> data;
				data.resize(unpack<int>(buffer));  // Tamaño del contenido
				unpackv(buffer, (char*)data.data(), (int)data.size());  // Contenido

				// INVOCAR: Escribir en el sistema de archivos
				instanciasFileManager[clientId].writeFile(file, data);

				// CONFIRMAR: Operación completada
				buffer.clear();
				pack(buffer,ack);
				sendMSG(clientId,buffer);
			}break;

			default:
				break;
		}
	} while(!logOut);  // Continuar hasta que el cliente haga logout

	// ==================== LIMPIEZA FINAL ====================
	// CERRAR: Terminar conexión TCP con el cliente
	// IMPORTANTE: Esto libera el socket y recursos asociados
	closeConnection(clientId);
	// NOTA: El thread termina aquí, se autodestruye (detach())
}