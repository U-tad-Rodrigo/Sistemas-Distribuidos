# DIAGRAMAS DE FLUJO Y SECUENCIA - SISTEMAS DISTRIBUIDOS

## 📊 DIAGRAMA DE FLUJO COMPLETO DEL SISTEMA

```
┌─────────────────────────────────────────────────────────────────┐
│                         ARQUITECTURA GENERAL                     │
└─────────────────────────────────────────────────────────────────┘

    ┌─────────┐                ┌─────────┐                ┌─────────┐
    │ BROKER  │                │ SERVER  │                │ CLIENT  │
    │ :1066   │                │ :1067   │                │         │
    └────┬────┘                └────┬────┘                └────┬────┘
         │                          │                          │
         │◄─────(1) REGISTER────────┤                          │
         │      SERVER_CONNECT       │                          │
         │      + IP:PORT           │                          │
         ├──────(2) ACK_BROKER──────►│                          │
         │      [CIERRA CONEXIÓN]    │                          │
         │                          │                          │
         │                          ├──(3) KEEP-ALIVE LOOP─┐   │
         │                          │    (cada 5 seg)      │   │
         │◄─────(4) ACK_BROKER──────┤                      │   │
         │      + serverID          │◄─────────────────────┘   │
         ├──────(5) ACK_BROKER──────►│                          │
         │      [CIERRA CONEXIÓN]    │                          │
         │                          │                          │
         │◄─────(6) CLIENT_CONNECT───────────────────────────────┤
         │                          │                          │
         ├──────(7) ACK_BROKER──────────────────────────────────►│
         │      + SERVER IP:PORT    │                          │
         │      [CIERRA CONEXIÓN]   │                          │
         │                          │                          │
         │                          │◄─────(8) CONNECT─────────┤
         │                          │      [PERSISTENTE]       │
         │                          │                          │
         │                          │◄─────(9) constructor─────┤
         │                          ├──────(10) ack────────────►│
         │                          │                          │
         │                          │◄─────(11) listFiles──────┤
         │                          ├──────(12) [files]────────►│
         │                          │                          │
         │                          │◄─────(13) readFile───────┤
         │                          ├──────(14) [data]─────────►│
         │                          │                          │
         │                          │◄─────(15) writeFile──────┤
         │                          ├──────(16) ack────────────►│
         │                          │                          │
         │                          │◄─────(17) destructor─────┤
         │                          ├──────(18) ack────────────►│
         │                          │      [CIERRA CONEXIÓN]   │
         │                          │                          │
```

---

## 🔄 SECUENCIA DETALLADA: REGISTRO DE SERVIDOR

```
SERVER                         BROKER
  │                              │
  │────(1) connect()────────────►│
  │    [Abrir socket TCP]        │
  │                              │
  │────(2) SERVER_CONNECT───────►│
  │    pack(SERVER_CONNECT)      │
  │    pack(IP.size)             │
  │    packv(IP.data)            │
  │    pack(PORT)                │
  │    sendMSG()                 │
  │                              │
  │                              ├─(3) Procesar
  │                              │   unpack(tipo)
  │                              │   unpack(IP)
  │                              │   unpack(PORT)
  │                              │   
  │                              ├─(4) Registrar
  │                              │   serverInfo.ip = IP
  │                              │   serverInfo.port = PORT
  │                              │   serverInfo.numClients = 0
  │                              │   serverInfo.keepAliveCounter = 0
  │                              │   serverInfo.alive = true
  │                              │   servidoresRegistrados[connId] = serverInfo
  │                              │
  │◄───(5) ACK_BROKER────────────┤
  │    recvMSG()                 │
  │    unpack(ACK_BROKER)        │
  │                              │
  │                              ├─(6) closeConnection()
  │◄───(7) [SOCKET CERRADO]──────┤
  │                              │
  ├─(8) Lanzar thread            │
  │   keepAlive()                │
  │                              │
  ├─(9) initServer(1067)         │
  │   Abrir puerto para clientes │
  │                              │
  └─(10) LISTO                   │
```

---

## 💓 SECUENCIA DETALLADA: KEEP-ALIVE

```
SERVER                         BROKER                    MONITOR THREAD
  │                              │                              │
  │                              │                              ├─(cada 5 seg)
  │                              │                              │  for each server:
  │                              │                              │    counter++
  │                              │                              │    if counter > MAX:
  │                              │                              │      alive = false
  │                              │                              │
  ├─(cada 5 seg)                 │                              │
  │  sleep(5)                    │                              │
  │                              │                              │
  │────(1) connect()────────────►│                              │
  │    [Nueva conexión]          │                              │
  │                              │                              │
  │────(2) ACK_BROKER───────────►│                              │
  │    pack(ACK_BROKER)          │                              │
  │    pack(brokerConnectionId)  │◄─ID original del registro   │
  │    sendMSG()                 │                              │
  │                              │                              │
  │                              ├─(3) Procesar                 │
  │                              │   tipo = unpack()            │
  │                              │   serverID = unpack()        │
  │                              │                              │
  │                              ├─(4) RESETEAR                 │
  │                              │   lock(mutex)                │
  │                              │   servidores[serverID]       │
  │                              │     .keepAliveCounter = 0    │
  │                              │   servidores[serverID]       │
  │                              │     .alive = true            │
  │                              │   unlock(mutex)              │
  │                              │                              │
  │◄───(5) ACK_BROKER────────────┤                              │
  │                              │                              │
  │◄───(6) [CIERRA CONEXIÓN]─────┤                              │
  │                              │                              │
  └─(7) Esperar 5 seg y repetir  │                              │
```

**IMPORTANTE**: Si el servidor NO envía keep-alive, el monitor incrementará
el contador hasta > MAX y marcará alive=false.

---

## 🔍 SECUENCIA DETALLADA: CLIENTE SOLICITA SERVIDOR

```
CLIENT                         BROKER
  │                              │
  │────(1) connect()────────────►│
  │    [Conectar a :1066]        │
  │                              │
  │────(2) CLIENT_CONNECT───────►│
  │    pack(CLIENT_CONNECT)      │
  │    sendMSG()                 │
  │                              │
  │                              ├─(3) Procesar
  │                              │   tipo = unpack()
  │                              │
  │                              ├─(4) Buscar servidor
  │                              │   findServerWithLessClients():
  │                              │     lock(mutex)
  │                              │     for each server:
  │                              │       if alive && numClients < min:
  │                              │         selected = server
  │                              │     unlock(mutex)
  │                              │     return selected
  │                              │
  │                              ├─(5) Incrementar carga
  │                              │   lock(mutex)
  │                              │   servidores[selected].numClients++
  │                              │   unlock(mutex)
  │                              │
  │◄───(6) ACK_BROKER────────────┤
  │    recvMSG()                 │
  │    unpack(ACK_BROKER)        │
  │    unpack(IP.size)           │
  │    unpackv(IP.data)          │
  │    unpack(PORT)              │
  │                              │
  │◄───(7) [CIERRA CONEXIÓN]─────┤
  │                              │
  ├─(8) Guardar                  │
  │   assignedServerIp = IP      │
  │   assignedServerPort = PORT  │
  │                              │
  └─(9) LISTO                    │
```

---

## 📞 SECUENCIA DETALLADA: OPERACIÓN RPC (listFiles)

```
CLIENT                         SERVER
  │                              │
  │────(1) listFiles()           │ ◄─ Llamada local
  │                              │
  ├─(2) Serializar               │
  │   buffer.clear()             │
  │   pack(listFilesF)           │
  │                              │
  │────(3) sendMSG()────────────►│
  │    [Transmitir por TCP]      │
  │                              │
  │                              ├─(4) Recibir
  │                              │   recvMSG(buffer)
  │                              │
  │                              ├─(5) Deserializar
  │                              │   tipo = unpack()
  │                              │
  │                              ├─(6) Switch
  │                              │   case listFilesF:
  │                              │
  │                              ├─(7) Ejecutar local
  │                              │   resultado = 
  │                              │     instancias[clientId]
  │                              │       .listFiles()
  │                              │
  │                              ├─(8) Serializar respuesta
  │                              │   buffer.clear()
  │                              │   pack(resultado.size())
  │                              │   for each file:
  │                              │     pack(file.size())
  │                              │     packv(file.data())
  │                              │
  │◄───(9) sendMSG()─────────────┤
  │    [Transmitir resultado]    │
  │                              │
  ├─(10) Recibir                 │
  │   recvMSG(buffer)            │
  │                              │
  ├─(11) Deserializar            │
  │   numFiles = unpack()        │
  │   for i in numFiles:         │
  │     size = unpack()          │
  │     file = unpackv()         │
  │     resultado.push(file)     │
  │                              │
  └─(12) return resultado        │ ◄─ Retorno local
```

---

## 🧵 DIAGRAMA DE THREADS

```
┌──────────────────────────────────────────────────────────────┐
│                          BROKER                               │
└──────────────────────────────────────────────────────────────┘

    ┌─────────────────┐
    │   MAIN THREAD   │
    │   - initServer  │
    │   - Bucle while │
    │   - getLastClientID()
    └────────┬────────┘
             │
             ├─────────────────────────────────────────┐
             │                                         │
    ┌────────▼────────┐                    ┌──────────▼────────────┐
    │ KEEP-ALIVE      │                    │ ACCEPT CONNECTIONS    │
    │ MONITOR THREAD  │                    │ THREAD (background)   │
    │ - while(true)   │                    │ - waitForConnectionsAsync()
    │ - sleep(5)      │                    │ - accept() [BLOQUEA]  │
    │ - Incrementar   │                    │ - Registrar cliente   │
    │   counters      │                    │ - waitingClients.push │
    │ - Marcar muertos│                    └───────────────────────┘
    └─────────────────┘
             
             │
             └─────────────────┬──────────────────┬─────────────────┐
                              │                  │                 │
                   ┌──────────▼──────────┐  ┌────▼────────┐  ┌────▼────────┐
                   │ CONNECTION THREAD 1 │  │ CONN THR 2  │  │ CONN THR N  │
                   │ - resolveBrokerMsgs │  │ - resolve.. │  │ - resolve.. │
                   │ - recvMSG()         │  │ - recvMSG() │  │ - recvMSG() │
                   │ - switch(tipo)      │  │ - switch()  │  │ - switch()  │
                   │ - sendMSG()         │  │ - sendMSG() │  │ - sendMSG() │
                   │ - closeConnection() │  │ - close()   │  │ - close()   │
                   └─────────────────────┘  └─────────────┘  └─────────────┘

┌──────────────────────────────────────────────────────────────┐
│                         SERVER                                │
└──────────────────────────────────────────────────────────────┘

    ┌─────────────────┐
    │   MAIN THREAD   │
    │   - Registrar   │
    │     en broker   │
    │   - initServer  │
    │   - Bucle while │
    └────────┬────────┘
             │
             ├─────────────────────────────────────────┐
             │                                         │
    ┌────────▼────────┐                    ┌──────────▼────────────┐
    │ KEEP-ALIVE      │                    │ ACCEPT CONNECTIONS    │
    │ THREAD          │                    │ THREAD                │
    │ - while(true)   │                    │ - waitForConnectionsAsync()
    │ - sleep(5)      │                    │ - accept()            │
    │ - Conectar      │                    │ - Registrar cliente   │
    │   broker        │                    └───────────────────────┘
    │ - Enviar ACK    │
    │ - Cerrar        │
    └─────────────────┘
             
             │
             └─────────────────┬──────────────────┬─────────────────┐
                              │                  │                 │
                   ┌──────────▼──────────┐  ┌────▼────────┐  ┌────▼────────┐
                   │ CLIENT HANDLER 1    │  │ HANDLER 2   │  │ HANDLER N   │
                   │ - resolveClientMsgs │  │ - resolve.. │  │ - resolve.. │
                   │ - Bucle do-while    │  │ - Bucle..   │  │ - Bucle..   │
                   │ - recvMSG()         │  │ - recvMSG() │  │ - recvMSG() │
                   │ - switch(tipo)      │  │ - switch()  │  │ - switch()  │
                   │ - Invocar FileMan   │  │ - Invocar.. │  │ - Invocar.. │
                   │ - sendMSG()         │  │ - sendMSG() │  │ - sendMSG() │
                   └─────────────────────┘  └─────────────┘  └─────────────┘
```

---

## 🔐 SECCIONES CRÍTICAS (MUTEX)

```
┌────────────────────────────────────────────────────────────────┐
│  PROBLEMA: Múltiples threads acceden a servidoresRegistrados   │
└────────────────────────────────────────────────────────────────┘

    Thread 1                    servidoresRegistrados            Thread 2
    (Registro)                   [MAPA COMPARTIDO]              (Consulta)
        │                               │                           │
        │                               │                           │
        ├─(1) lock_guard<mutex>(mutex) │                           │
        │   ◄────MUTEX ADQUIRIDO────────┤                           │
        │                               │                           │
        ├─(2) servidores[id] = info    │                           │
        │                               │                           │
        │                               │     ┌─────────────────────┤
        │                               │     │ lock_guard<mutex>   │
        │                               │     │ ◄─ESPERA (bloqueado)
        │                               │     │                     │
        ├─(3) sale del scope            │     │                     │
        │   ◄────MUTEX LIBERADO─────────┤     │                     │
        │                               │     │                     │
        │                               │◄────┘ MUTEX ADQUIRIDO    │
        │                               │                           │
        │                               │    ┌──(4) findServerWithLessClients()
        │                               │◄───┤  for (auto& pair...)
        │                               │    │                      │
        │                               │    └──(5) sale del scope  │
        │                               │◄────MUTEX LIBERADO────────┤
        │                               │                           │

RESULTADO: Nunca hay acceso simultáneo -> NO HAY CONDICIÓN DE CARRERA
```

---

## 📦 PROTOCOLO DE EMPAQUETADO/DESEMPAQUETADO

```
┌─────────────────────────────────────────────────────────────┐
│         EJEMPLO: Enviar mensaje SERVER_CONNECT              │
└─────────────────────────────────────────────────────────────┘

LADO EMISOR (Servidor):

    vector<unsigned char> buffer;  // Vacío: []
    
    pack(buffer, SERVER_CONNECT);  // [1, 0, 0, 0] (enum = 1, 4 bytes)
    
    string ip = "127.0.0.1";
    pack(buffer, (int)ip.size());  // [1,0,0,0, 9,0,0,0] (tamaño=9)
    
    packv(buffer, ip.data(), 9);   // [1,0,0,0, 9,0,0,0, '1','2','7','.','0','.','0','.','1']
    
    pack(buffer, 1067);            // [..., 43,4,0,0] (puerto=1067)
    
    sendMSG(connId, buffer);       // Envía: tamaño_total(int) + datos

LADO RECEPTOR (Broker):

    vector<unsigned char> buffer;  // Después de recvMSG()
    
    brokerMsgTypes tipo = unpack<brokerMsgTypes>(buffer);
    // tipo = SERVER_CONNECT
    // buffer ahora: [9,0,0,0, '1','2','7','.','0','.','0','.','1', 43,4,0,0]
    
    int ipSize = unpack<int>(buffer);
    // ipSize = 9
    // buffer ahora: ['1','2','7','.','0','.','0','.','1', 43,4,0,0]
    
    string ip;
    ip.resize(9);
    unpackv(buffer, ip.data(), 9);
    // ip = "127.0.0.1"
    // buffer ahora: [43,4,0,0]
    
    int port = unpack<int>(buffer);
    // port = 1067
    // buffer ahora: []
```

**REGLA DE ORO**: El orden de unpack() DEBE coincidir con el orden de pack()

---

## 🎯 ESTADOS DEL SERVIDOR EN EL BROKER

```
┌─────────────────────────────────────────────────────────────┐
│              MÁQUINA DE ESTADOS DE UN SERVIDOR              │
└─────────────────────────────────────────────────────────────┘

    ┌─────────────┐
    │   INICIAL   │
    │  (no existe)│
    └──────┬──────┘
           │
           │ SERVER_CONNECT
           │
    ┌──────▼──────┐
    │  REGISTRADO │
    │  alive=true │
    │  counter=0  │
    └──────┬──────┘
           │
           ├──────────────────┐
           │                  │
           │ KEEP-ALIVE       │ NO KEEP-ALIVE
           │ (counter=0)      │ (counter++)
           │                  │
    ┌──────▼──────┐    ┌──────▼──────┐
    │  SALUDABLE  │    │  TIMEOUT    │
    │  alive=true │    │  alive=false│
    │  counter=0  │    │  counter>MAX│
    └──────┬──────┘    └──────┬──────┘
           │                  │
           │ KEEP-ALIVE       │ KEEP-ALIVE
           │                  │ (recuperación)
           └─────────┬────────┘
                     │
              ┌──────▼──────┐
              │ REACTIVADO  │
              │ alive=true  │
              │ counter=0   │
              └─────────────┘

NOTA: El servidor NO se elimina del mapa cuando cae,
      solo se marca como no disponible (alive=false)
```

---

## 🔄 CICLO DE VIDA COMPLETO

```
┌──────────────────────────────────────────────────────────────────┐
│                     CICLO DE VIDA COMPLETO                        │
└──────────────────────────────────────────────────────────────────┘

1. INICIO
   ┌─────────┐
   │ BROKER  │ ← Arranca primero (puerto 1066)
   └─────────┘
        │
        ├── Lanza monitor keep-alive
        └── Espera conexiones
   
2. REGISTRO DE SERVIDORES
   ┌─────────┐
   │ SERVER1 │ ← Arranca, se registra, abre puerto 1067
   └─────────┘
   ┌─────────┐
   │ SERVER2 │ ← Arranca, se registra, abre puerto 1068
   └─────────┘
        │
        └── Ambos envían keep-alive periódicamente

3. CLIENTE SE CONECTA
   ┌─────────┐
   │ CLIENT  │ ← Pregunta al broker
   └─────────┘
        │
        ├── Broker asigna SERVER1 (menos clientes)
        │
        └── Cliente se conecta a SERVER1:1067

4. OPERACIONES
   CLIENT <──RPC──> SERVER1
        │
        ├── listFiles()
        ├── readFile("test.txt")
        └── writeFile("new.txt")

5. FALLO
   ┌─────────┐
   │ SERVER1 │ ← CRASH
   └─────────┘
        │
        ├── Deja de enviar keep-alive
        │
        └── Broker marca alive=false tras 10 seg (2 ciclos)

6. RECUPERACIÓN
   ┌─────────┐
   │ CLIENT2 │ ← Nuevo cliente pregunta al broker
   └─────────┘
        │
        └── Broker asigna SERVER2 (único disponible)

7. CIERRE
   CLIENT <─destructor─> SERVER2
        │
        └── Servidor cierra conexión, libera instancia
```

---

## 📚 RESUMEN DE PATRONES

```
┌──────────────────────────────────────────────────────────────┐
│                    PATRONES IMPLEMENTADOS                     │
└──────────────────────────────────────────────────────────────┘

1. BROKER/MEDIATOR
   Client ──┐
           ├──► BROKER ◄──┐
   Client ──┘              ├── Server1
                           ├── Server2
                           └── ServerN

2. RPC (Remote Procedure Call)
   Client.method() ──serializa──> Red ──deserializa──> Server.method()
                   ◄──serializa─── Red ◄──resultado──

3. THREAD PER CONNECTION
   Main ──┐
          ├──► Thread1 (connection1)
          ├──► Thread2 (connection2)
          └──► ThreadN (connectionN)

4. HEARTBEAT / KEEP-ALIVE
   Server ──(cada 5 seg)──> Broker
          ◄──────ACK────────
   [Si no llega: Broker marca como muerto]

5. LOAD BALANCING
   Broker: for each server:
             if alive && clients < min:
               selected = server
           assign client to selected

6. SERVICE DISCOVERY
   Client: "¿Dónde hay un servidor?"
   Broker: "Usa IP:PORT"
   Client: [conecta directamente]
```

---

¡Estudia estos diagramas junto con los comentarios del código! 🎓

