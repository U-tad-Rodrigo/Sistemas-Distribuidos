#pragma once

#include "utils.h"
#include <string>
#include "filemanager.h"

using namespace std;

typedef enum {
    MSG_CONSTRUCTOR,           // Crear instancia de FileManager
    MSG_LIST_FILES,           // Solicitar lista de archivos
    MSG_LIST_FILES_RESPONSE,  // Respuesta con lista de archivos
    MSG_READ_FILE,            // Solicitar leer archivo
    MSG_READ_FILE_RESPONSE,   // Respuesta con contenido del archivo
    MSG_WRITE_FILE,           // Solicitar escribir archivo
    MSG_WRITE_FILE_RESPONSE,  // Confirmación de escritura
    MSG_ACK,                  // Mensaje de confirmación genérico
    MSG_ERROR                 // Mensaje de error
} msgTypes;

class clientManager {
    static inline map<FileManager*, int> connectionIds;
    static inline map<int, FileManager> instanciasFileManager;

    static void atiendeCliente(int clientId);
};