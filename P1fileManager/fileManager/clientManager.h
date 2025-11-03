#pragma once

#include "utils.h"
#include <string>
#include "filemanager.h"

using namespace std;

typedef enum {
    constructorFilemanager,
    constructorFilemanagerParams,
    ack
}msgTypes;

class clientManager {
    static inline map<FileManager*, int> connectionIds;
    static inline map<int, FileManager> instanciasFileManager;

    static void atiendeCliente(int clientId);
};