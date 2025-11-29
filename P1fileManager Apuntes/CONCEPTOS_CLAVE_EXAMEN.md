# CONCEPTOS CLAVE PARA EL EXAMEN - SISTEMAS DISTRIBUIDOS

## 📚 ARQUITECTURA GENERAL DEL SISTEMA

### Componentes:
1. **BROKER** - Intermediario que conecta clientes con servidores
2. **SERVIDOR** - Procesa las peticiones de los clientes (FileManager)
3. **CLIENTE** - Utiliza los servicios remotos (distribuye FileManager)

---

## 🔄 FLUJO COMPLETO DE FUNCIONAMIENTO

### 1️⃣ INICIO DEL BROKER
```
Broker:
  - Abre puerto 1066
  - Lanza thread de keep-alive monitor
  - Espera conexiones (bucle infinito)
  - Por cada conexión: crea thread -> resolveBrokerMessages()
```

### 2️⃣ REGISTRO DEL SERVIDOR
```
Servidor:
  1. Se conecta al broker (puerto 1066)
  2. Envía: SERVER_CONNECT + su IP + su Puerto
  3. Broker registra: servidoresRegistrados[connectionId] = ServerInfo
  4. Broker responde: ACK_BROKER
  5. Broker CIERRA la conexión (efímera)
  6. Servidor lanza thread de keep-alive
  7. Servidor abre su puerto (1067) para clientes
```

### 3️⃣ KEEP-ALIVE (HEARTBEAT)
```
Servidor (cada 5 segundos):
  1. Nueva conexión al broker
  2. Envía: ACK_BROKER + su ID original
  3. Broker resetea keepAliveCounter[serverID] = 0
  4. Broker responde: ACK_BROKER
  5. Cierra conexión

Broker Monitor (cada 5 segundos):
  1. Incrementa keepAliveCounter de todos los servidores
  2. Si contador > KEEP_ALIVE_MAX_COUNT (2):
     - Marca servidor como alive=false
     - No lo elimina (puede recuperarse)
```

### 4️⃣ CLIENTE SOLICITA SERVIDOR
```
Cliente:
  1. Conecta al broker (puerto 1066)
  2. Envía: CLIENT_CONNECT
  3. Broker busca servidor con menos clientes (findServerWithLessClients)
  4. Broker responde: ACK_BROKER + IP del servidor + Puerto
  5. Broker incrementa numClients del servidor
  6. Cierra conexión con broker
```

### 5️⃣ CLIENTE USA EL SERVIDOR
```
Cliente:
  1. Conecta al servidor asignado (puerto 1067)
  2. Envía: constructorFilemanager o constructorFilemanagerParams
  3. Servidor crea instancia: instanciasFileManager[clientId]
  4. Servidor responde: ack
  5. CONEXIÓN PERSISTENTE (no se cierra)
  
  Operaciones:
  - listFilesF: Cliente pide lista -> Servidor responde vector<string>
  - readFileF: Cliente pide archivo -> Servidor responde contenido
  - writeFileF: Cliente envía datos -> Servidor escribe y responde ack
  
  Cierre:
  - Cliente envía: destructorFilemanager
  - Servidor elimina instancia
  - Servidor responde: ack
  - Servidor CIERRA la conexión
```

---

## 🔑 CONCEPTOS FUNDAMENTALES

### 🌐 PATRONES DE DISEÑO

#### 1. **BROKER / MEDIATOR**
- **Qué es**: Intermediario que desacopla clientes de servidores
- **Ventajas**: 
  - Clientes no necesitan conocer IPs de servidores
  - Balanceo de carga centralizado
  - Detección de fallos centralizada
- **Ubicación**: `broker.cpp`, `brokerManager.cpp`

#### 2. **RPC (Remote Procedure Call)**
- **Qué es**: Invocar funciones remotas como si fueran locales
- **Implementación**: 
  - Cliente llama `fm.listFiles()`
  - Se traduce a: pack mensaje -> enviar -> esperar respuesta -> desempaquetar
- **Ubicación**: `distributedFileManager.cpp`, `clientManager.cpp`

#### 3. **SERVICE DISCOVERY**
- **Qué es**: Mecanismo para que los clientes encuentren servidores
- **Implementación**: Cliente pregunta al broker "¿dónde hay un servidor?"
- **Ubicación**: `obtenerServidorDelBroker()` en `distributedFileManager.cpp`

#### 4. **LOAD BALANCING**
- **Qué es**: Distribuir carga entre múltiples servidores
- **Estrategia**: Round-robin basado en carga (servidor con menos clientes)
- **Ubicación**: `findServerWithLessClients()` en `brokerManager.cpp`

---

### 🔧 MECANISMOS TÉCNICOS

#### 1. **SERIALIZACIÓN (MARSHALLING)**
- **Qué es**: Convertir datos de C++ a bytes para transmitir
- **Funciones clave**:
  ```cpp
  pack(buffer, dato)    // Añadir dato al buffer
  unpack(buffer)        // Extraer dato del buffer
  packv(buffer, array)  // Añadir array
  unpackv(buffer, array) // Extraer array
  ```
- **Protocolo**: Para strings: `tamaño (int) + datos (chars)`
- **Ubicación**: `utils.h` (templates inline)

#### 2. **PROTOCOLO DE MENSAJES**
- **Estructura**: Siempre empieza con el tipo de mensaje (enum)
  ```cpp
  buffer.clear();
  pack(buffer, TIPO_MENSAJE);  // Primero el tipo
  pack(buffer, parametro1);     // Luego parámetros
  pack(buffer, parametro2);
  sendMSG(connectionId, buffer);
  ```
- **Recepción**:
  ```cpp
  recvMSG(connectionId, buffer);
  MsgType tipo = unpack<MsgType>(buffer);  // Primero leer tipo
  // Luego desempaquetar parámetros según el tipo
  ```

#### 3. **COMUNICACIÓN TCP**
- **sendMSG**: 
  1. Envía tamaño del mensaje (4 bytes int)
  2. Envía los datos
- **recvMSG**: 
  1. Lee tamaño (4 bytes)
  2. Lee datos hasta completar el tamaño
  3. IMPORTANTE: Puede requerir múltiples llamadas a `read()`

#### 4. **KEEP-ALIVE / HEARTBEAT**
- **Objetivo**: Detectar servidores caídos
- **Mecanismo**:
  - Servidor envía señal cada X segundos
  - Broker tiene un contador por servidor
  - Si contador > MAX sin recibir señal -> servidor caído
- **Ventaja**: Detecta fallos sin esperar a que un cliente se conecte

#### 5. **CONCURRENCIA (MULTITHREADING)**
- **Patrón**: Un thread por conexión
- **Ventajas**: 
  - Broker puede atender múltiples clientes simultáneamente
  - Servidor puede procesar múltiples peticiones en paralelo
- **Sincronización**: 
  ```cpp
  lock_guard<mutex> lock(serversMutex);  // Protege mapa compartido
  // ... acceso a servidoresRegistrados ...
  // mutex se desbloquea automáticamente al salir del scope
  ```

#### 6. **CONEXIONES EFÍMERAS vs PERSISTENTES**
- **Efímeras** (Broker):
  - Abrir -> enviar mensaje -> recibir respuesta -> cerrar
  - Ventaja: No mantiene recursos abiertos innecesariamente
  - Uso: Registro, keep-alive, consulta de servidor
- **Persistentes** (Cliente-Servidor):
  - Abrir -> múltiples operaciones -> cerrar al terminar
  - Ventaja: No re-establecer conexión en cada operación
  - Uso: Sesión completa del cliente

---

## 📦 TIPOS DE MENSAJES

### BROKER (brokerMsgTypes):
```cpp
SERVER_CONNECT   // Servidor se registra
CLIENT_CONNECT   // Cliente pide servidor
ACK_BROKER       // Confirmación / Keep-alive
```

### CLIENTE-SERVIDOR (msgTypes):
```cpp
constructorFilemanager        // Crear instancia sin path
constructorFilemanagerParams  // Crear instancia con path
destructorFilemanager         // Destruir instancia
listFilesF                    // Listar archivos
readFileF                     // Leer archivo
writeFileF                    // Escribir archivo
ack                          // Confirmación
```

---

## 🗺️ ESTRUCTURAS DE DATOS CLAVE

### 1. ServerInfo (Broker)
```cpp
struct ServerInfo {
    string ip;              // IP del servidor
    int port;               // Puerto del servidor
    int numClients;         // Contador de clientes (balanceo)
    int keepAliveCounter;   // Ciclos sin respuesta (detección fallos)
    bool alive;             // Estado del servidor
}
```

### 2. connection_t (Utils)
```cpp
struct connection_t {
    unsigned int id;        // ID único de la conexión
    unsigned int serverId;  // ID local del socket
    int socket;             // Descriptor del socket TCP
    list<msg_t*>* buffer;   // Buffer para modo asíncrono
    bool alive;             // Estado de la conexión
}
```

### 3. Mapas importantes:
```cpp
// BROKER: ID conexión -> Info del servidor
map<int, ServerInfo> servidoresRegistrados;

// SERVIDOR: ID cliente -> Instancia de FileManager
map<int, FileManager> instanciasFileManager;

// CLIENTE: Objeto FileManager -> ID conexión con servidor
map<FileManager*, int> connectionIds;

// GLOBAL: ID conexión -> Estructura de conexión
map<unsigned int, connection_t> clientList;
```

---

## 🚀 FUNCIONES DE RED IMPORTANTES

### Servidor (initServer):
```cpp
socket()      // Crear socket
bind()        // Asociar a puerto
listen()      // Modo escucha
accept()      // Aceptar conexión (bloquea hasta que llega cliente)
```

### Cliente (initClient):
```cpp
socket()      // Crear socket
connect()     // Conectar a servidor (handshake TCP)
```

### Ambos:
```cpp
read()        // Leer datos del socket
write()       // Escribir datos al socket
close()       // Cerrar socket
```

---

## ⚠️ ERRORES COMUNES Y SOLUCIONES

### 1. Condición de carrera (Race Condition)
- **Problema**: Múltiples threads modifican misma variable
- **Solución**: `lock_guard<mutex> lock(mutex_var);`

### 2. Buffer no completo
- **Problema**: `read()` puede retornar menos bytes de los pedidos
- **Solución**: Bucle hasta leer todos los bytes esperados

### 3. Orden de empaquetado/desempaquetado
- **Problema**: Desempaquetar en orden diferente al empaquetado
- **Solución**: Mantener mismo orden: pack(A), pack(B) -> unpack<A>(), unpack<B>()

### 4. Memory leaks con threads
- **Problema**: `new thread` sin liberar
- **Solución**: `detach()` para que se autodestruya

### 5. Servidor cae y broker no lo detecta
- **Problema**: Keep-alive no funciona o intervalo muy largo
- **Solución**: Verificar que servidor envía keep-alive y broker lo procesa

---

## 🎯 PREGUNTAS TÍPICAS DE EXAMEN

### 1. ¿Qué pasa si un servidor cae?
- El monitor de keep-alive detecta falta de señales
- Marca `alive=false` tras KEEP_ALIVE_MAX_COUNT ciclos
- Broker ya no asigna clientes nuevos a ese servidor
- Clientes ya conectados pierden conexión

### 2. ¿Cómo se hace balanceo de carga?
- `findServerWithLessClients()` itera servidores vivos
- Selecciona el que tiene menor `numClients`
- Al asignar, incrementa su contador

### 3. ¿Por qué usar threads?
- Procesar múltiples conexiones simultáneamente
- No bloquear el main mientras se atiende un cliente
- Broker puede registrar servidor mientras atiende cliente

### 4. ¿Qué es el marshalling?
- Convertir datos de C++ a bytes (serialización)
- Necesario para enviar por red (sockets transmiten bytes)
- `pack()` serializa, `unpack()` deserializa

### 5. ¿Diferencia entre conexión efímera y persistente?
- **Efímera**: Abrir-usar-cerrar inmediatamente (broker)
- **Persistente**: Mantener abierta para múltiples operaciones (cliente-servidor)

---

## 🔄 ADAPTACIÓN A OTROS SERVICIOS (Database, etc.)

### Si en vez de FileManager fuera Database:

1. **Cambiar mensajes**:
   ```cpp
   enum msgTypes {
       queryDB,      // En vez de readFileF
       insertDB,     // En vez de writeFileF
       updateDB,     // Nuevo
       deleteDB,     // Nuevo
       ack
   }
   ```

2. **Cambiar instancia**:
   ```cpp
   map<int, Database> instanciasDatabase;  // En vez de FileManager
   ```

3. **Mantener igual**:
   - Broker (no cambia, solo conecta)
   - Keep-alive (mismo mecanismo)
   - Serialización (pack/unpack igual)
   - Comunicación TCP (igual)
   - Threads y concurrencia (igual)

**CONCLUSIÓN**: La arquitectura es genérica, solo cambian los mensajes específicos del servicio.

---

## 📝 CHECKLIST PARA EL EXAMEN

- [ ] Entiendo el flujo completo: Broker -> Servidor -> Cliente
- [ ] Sé explicar keep-alive y detección de fallos
- [ ] Entiendo balanceo de carga (findServerWithLessClients)
- [ ] Sé usar pack/unpack para serialización
- [ ] Entiendo el protocolo de mensajes (tipo + parámetros)
- [ ] Sé cuándo usar mutex (acceso a variables compartidas)
- [ ] Entiendo conexiones efímeras vs persistentes
- [ ] Sé adaptar mensajes para otro servicio (Database, etc.)
- [ ] Entiendo thread por conexión
- [ ] Sé explicar RPC y Service Discovery

---

## 🎓 CONSEJOS FINALES

1. **Orden de mensajes**: SIEMPRE tipo primero, luego parámetros
2. **Strings en red**: Siempre `tamaño + datos`
3. **Mutex**: Si múltiples threads acceden a misma variable -> mutex
4. **Threads**: Un thread por conexión es el patrón estándar
5. **Keep-alive**: Mecanismo esencial para sistemas distribuidos reales
6. **Broker**: No procesa lógica de negocio, solo conecta componentes

**¡Éxito en el examen! 🚀**

