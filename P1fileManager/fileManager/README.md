# Práctica 1 - FileManager Distribuido

## Estructura del Proyecto

### Archivos Proporcionados (NO MODIFICAR)
- **filemanager.h**: Interfaz de la clase FileManager
- **libFileManager.a**: Librería compilada con la implementación
- **main_fm.cpp**: Programa de prueba del FileManager local

### Archivos a Implementar

#### Servidor (`server.cpp`)
- Iniciar un servidor en un puerto específico
- Esperar conexiones de clientes
- Crear instancias de FileManager para cada cliente
- Procesar peticiones remotas (listFiles, readFile, writeFile)
- Enviar respuestas a los clientes

#### Cliente (`client.cpp`)
- Conectarse al servidor
- Enviar peticiones de operaciones FileManager
- Recibir y procesar respuestas
- Idealmente, mantener la misma interfaz que `main_fm.cpp`

#### ClientManager (`clientManager.h/cpp`)
- Gestionar las conexiones cliente-servidor
- Mantener el mapeo entre clientes y sus instancias de FileManager
- Procesar mensajes y traducir entre protocolo de red y llamadas locales

#### DistributedFileManager (`distributedFileManager.cpp`)
- Implementar un proxy o wrapper del FileManager
- Enviar peticiones al servidor en lugar de ejecutar localmente
- Hacer transparente la distribución para el cliente

### Utilidades Proporcionadas (`utils.h/cpp`)
Funciones de red disponibles:
- `initServer(int port)`: Inicializar servidor
- `initClient(string host, int port)`: Conectar como cliente
- `waitForConnections(int sock_fd)`: Esperar nueva conexión
- `sendMSG<T>(int clientID, vector<T> &data)`: Enviar mensaje
- `recvMSG<T>(int clientID, vector<T> &data)`: Recibir mensaje
- `closeConnection(int clientID)`: Cerrar conexión

### Compilación
```bash
mkdir build
cd build
cmake ..
make
```

Esto generará tres ejecutables:
- `fileManager`: Programa de prueba local (funcional)
- `server`: Servidor del FileManager distribuido (implementar)
- `client`: Cliente del FileManager distribuido (implementar)

### Protocolo de Comunicación (a diseñar)
Define tu propio protocolo de mensajes. Ejemplo:
1. Cliente envía tipo de operación (listFiles/readFile/writeFile)
2. Cliente envía parámetros necesarios
3. Servidor procesa y envía respuesta
4. Cliente recibe y muestra resultado

### TODOs Principales
1. ✅ Estructura base del proyecto configurada
2. ⬜ Definir protocolo de mensajes (enum msgTypes en clientManager.h)
3. ⬜ Implementar servidor que cree FileManager y atienda peticiones
4. ⬜ Implementar cliente que envíe peticiones y reciba respuestas
5. ⬜ Probar con el ejecutable fileManager como referencia

### Directorio de Pruebas
Asegúrate de crear el directorio `FileManagerDir` en la ubicación donde ejecutes los programas.

## Notas
- El ejecutable `fileManager` (main_fm.cpp) funciona correctamente y puede usarse como referencia
- NO se pueden modificar los archivos proporcionados
- Todos los archivos deben incluir tu nombre en los comentarios

