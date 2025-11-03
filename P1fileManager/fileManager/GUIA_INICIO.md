# Guía de Inicio Rápido - Práctica 1

## Estado del Proyecto: ✅ BASE CONFIGURADA CORRECTAMENTE

### ✅ Correcciones Realizadas

1. **CMakeLists.txt corregido**
   - Eliminadas declaraciones duplicadas de `project()`
   - `client` ahora usa `client.cpp` (no `main_fm.cpp`)
   - `server` usa `server.cpp`
   - `fileManager` usa `main_fm.cpp` (programa de prueba original)

2. **Estructura de archivos**
   - ✅ `server.cpp`: Plantilla básica para el servidor
   - ✅ `client.cpp`: Plantilla básica para el cliente
   - ✅ `clientManager.cpp`: Función `atiendeCliente()` definida
   - ✅ `distributedFileManager.cpp`: Archivo creado para proxy/wrapper
   - ✅ `clientManager.h`: Enum `msgTypes` ampliado con protocolo completo

3. **Archivos de ayuda creados**
   - ✅ `README.md`: Documentación del proyecto
   - ✅ `EJEMPLOS.cpp`: Ejemplos de código comentados
   - ✅ `FileManagerDir/test.txt`: Archivo de prueba

## Próximos Pasos

### 1. Probar el programa original
Antes de empezar, compila y prueba el `fileManager` original:
```bash
cd cmake-build-debug-aws-ubuntu  # o crea tu propio build dir
cmake ..
make fileManager
./fileManager
```

Comandos disponibles:
- `ls`: Lista archivos locales
- `lls`: Lista archivos en FileManagerDir
- `upload nombre_archivo`: Sube archivo a FileManagerDir
- `download nombre_archivo`: Descarga archivo de FileManagerDir
- `exit()`: Salir

### 2. Implementar el servidor (server.cpp)
- Iniciar servidor con `initServer(puerto)`
- Esperar conexiones con `waitForConnections()` o `checkClient()` + `getLastClientID()`
- Para cada cliente, crear un thread que ejecute `clientManager::atiendeCliente(clientId)`

### 3. Implementar clientManager::atiendeCliente() (clientManager.cpp)
- Crear una instancia de `FileManager` para el cliente
- En un bucle:
  - Recibir tipo de operación con `recvMSG()`
  - Según el tipo, ejecutar la operación en el FileManager local
  - Enviar respuesta con `sendMSG()`

### 4. Implementar el cliente (client.cpp)
- Conectar al servidor con `initClient(host, puerto)`
- Replicar la lógica de `main_fm.cpp` pero:
  - En lugar de llamar a `fm.listFiles()`, enviar `MSG_LIST_FILES` al servidor
  - En lugar de llamar a `fm.readFile()`, enviar `MSG_READ_FILE` al servidor
  - En lugar de llamar a `fm.writeFile()`, enviar `MSG_WRITE_FILE` al servidor

## Protocolo de Comunicación

### Operación: Listar archivos
**Cliente → Servidor:**
```cpp
vector<int> msg = {MSG_LIST_FILES};
sendMSG(clientId, msg);
```

**Servidor → Cliente:**
```cpp
vector<int> numFiles = {files.size()};
sendMSG(clientId, numFiles);
// Para cada archivo:
vector<char> filename(name.begin(), name.end());
sendMSG(clientId, filename);
```

### Operación: Leer archivo
**Cliente → Servidor:**
```cpp
vector<int> msg = {MSG_READ_FILE};
sendMSG(clientId, msg);
vector<char> filename(name.begin(), name.end());
sendMSG(clientId, filename);
```

**Servidor → Cliente:**
```cpp
vector<unsigned char> data;
fm.readFile(filename, data);
sendMSG(clientId, data);
```

### Operación: Escribir archivo
**Cliente → Servidor:**
```cpp
vector<int> msg = {MSG_WRITE_FILE};
sendMSG(clientId, msg);
vector<char> filename(name.begin(), name.end());
sendMSG(clientId, filename);
sendMSG(clientId, data);  // vector<unsigned char>
```

**Servidor → Cliente:**
```cpp
vector<int> ack = {MSG_ACK};
sendMSG(clientId, ack);
```

## Funciones Útiles de utils.h

- `int initServer(int port)`: Inicializa servidor en puerto
- `connection_t initClient(string host, int port)`: Conecta como cliente
- `int waitForConnections(int sock_fd)`: Espera nueva conexión (bloqueante)
- `bool checkClient()`: Verifica si hay clientes esperando (no bloqueante)
- `int getLastClientID()`: Obtiene ID del último cliente conectado
- `void sendMSG<T>(int clientID, vector<T> &data)`: Envía mensaje
- `void recvMSG<T>(int clientID, vector<T> &data)`: Recibe mensaje (bloqueante)
- `void closeConnection(int clientID)`: Cierra conexión

## Consejos

1. **Empieza simple**: Implementa solo `listFiles` primero, luego añade el resto
2. **Prueba incrementalmente**: Compila y prueba después de cada función
3. **Usa el fileManager original**: Es tu referencia de cómo debe funcionar
4. **Debug con prints**: Usa `cout` para ver qué mensajes se envían/reciben
5. **Convierte tipos correctamente**: 
   - `string → vector<char>`: `vector<char> v(str.begin(), str.end())`
   - `vector<char> → string`: `string s(vec.begin(), vec.end())`

## Compilación y Prueba

### Opción 1: Usar el build existente
```bash
cd cmake-build-debug-aws-ubuntu
make
```

### Opción 2: Crear nuevo build
```bash
mkdir build
cd build
cmake ..
make
```

### Ejecutar
```bash
# Terminal 1 - Servidor
./server

# Terminal 2 - Cliente
./client

# Terminal 3 - FileManager original (para comparar)
./fileManager
```

## Recuerda
- ❌ NO modificar: `filemanager.h`, `libFileManager.a`, `main_fm.cpp`
- ✅ SÍ modificar: `server.cpp`, `client.cpp`, `clientManager.cpp/h`, `distributedFileManager.cpp`
- ✅ Añadir tu nombre en los comentarios de cada archivo
- ✅ El directorio `FileManagerDir` debe existir donde ejecutes los programas

