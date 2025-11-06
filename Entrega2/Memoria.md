# Memoria - Chat Cliente-Servidor

## Descripción

Sistema de chat básico con arquitectura cliente-servidor usando TCP. Permite que múltiples clientes se conecten simultáneamente y se comuniquen entre sí.

### Funcionalidades

- Múltiples clientes conectados al mismo tiempo (hilos independientes)
- Mensajes públicos (broadcast a todos los clientes)
- Mensajes privados con `@ID mensaje`
- Comando `usuarios` para listar clientes conectados
- Apagado ordenado del servidor con Ctrl+C (SIGINT)
- Desconexión limpia de clientes con `exit`

## Ejecución del programa

### 1. Arrancar el servidor

```
$ ./server
Servidor TCP Multi-Cliente
Puerto: 5000
Presiona Ctrl+C para apagar
----------------------------
Servidor iniciado, esperando conexiones...
```

El servidor queda esperando conexiones de clientes.

### 2. Conectar clientes

Abrir 3 terminales diferentes y ejecutar el cliente en cada una:

**Terminal Cliente 0:**
```
$ ./client
Cliente TCP
Conectando a 127.0.0.1:5000...
Intentando crear socket...
Socket creado: 3
Convirtiendo direccion IP...
Intentando conectar a 127.0.0.1:5000...
Conexion TCP establecida exitosamente
Cliente registrado con ID: 0
Conectado!
Comandos: 'usuarios', 'exit', '@ID mensaje' (privado)
----------------------------
> 
```

**Terminal Cliente 1:**
```
$ ./client
Cliente TCP
Conectando a 127.0.0.1:5000...
Intentando crear socket...
Socket creado: 3
Convirtiendo direccion IP...
Intentando conectar a 127.0.0.1:5000...
Conexion TCP establecida exitosamente
Cliente registrado con ID: 1
Conectado!
Comandos: 'usuarios', 'exit', '@ID mensaje' (privado)
----------------------------
> 
```

**Terminal Cliente 2:**
```
$ ./client
Cliente TCP
Conectando a 127.0.0.1:5000...
Intentando crear socket...
Socket creado: 3
Convirtiendo direccion IP...
Intentando conectar a 127.0.0.1:5000...
Conexion TCP establecida exitosamente
Cliente registrado con ID: 2
Conectado!
Comandos: 'usuarios', 'exit', '@ID mensaje' (privado)
----------------------------
> 
```

### 3. Servidor tras las conexiones

```
$ ./server
Servidor TCP Multi-Cliente
Puerto: 5000
Presiona Ctrl+C para apagar
----------------------------
Servidor iniciado, esperando conexiones...
[SERVIDOR] Cliente 0 conectado
[SERVIDOR] Cliente 1 conectado
[SERVIDOR] Cliente 2 conectado
```

### 4. Conversación entre clientes

**Terminal Servidor:**
```
[SERVIDOR] Cliente 0 conectado
[SERVIDOR] Cliente 1 conectado
[SERVIDOR] Cliente 2 conectado
cliente 0: Hola a todos!
cliente 1: Hola desde cliente 1
cliente 2: Saludos desde cliente 2
[SERVIDOR] Cliente 2 desconectado
[SERVIDOR] Cliente 1 desconectado
[SERVIDOR] Cliente 0 desconectado
```

**Terminal Cliente 0:**
```
> Hola a todos!
Servidor: mensaje recibido correctamente.
> cliente 1: Hola desde cliente 1
cliente 2: Saludos desde cliente 2
usuarios
conectados: 0,1,2
> @2 Este mensaje es privado para ti
Servidor: mensaje privado enviado a cliente 2
> exit
Cerrando conexion...
Desconectado.
```

**Terminal Cliente 1:**
```
> cliente 0: Hola a todos!
Hola desde cliente 1
Servidor: mensaje recibido correctamente.
> cliente 2: Saludos desde cliente 2
exit
Cerrando conexion...
Desconectado.
```

**Terminal Cliente 2:**
```
> cliente 0: Hola a todos!
cliente 1: Hola desde cliente 1
Saludos desde cliente 2
Servidor: mensaje recibido correctamente.
> [PRIVADO] cliente 0: Este mensaje es privado para ti
exit
Cerrando conexion...
Desconectado.

```

## Funcionalidades implementadas

### 1. Mensajes públicos (broadcast)

Cuando un cliente envía un mensaje normal, el servidor lo reenvía a todos los demás clientes conectados (excepto al emisor). El formato es:
```
cliente ID: mensaje
```

### 2. Mensajes privados

Para enviar un mensaje privado a un cliente específico, se usa:
```
@ID mensaje
```

Ejemplo: `@2 Hola, esto es privado`

El servidor busca al cliente con ese ID y le envía el mensaje solo a él. El receptor lo ve con el prefijo `[PRIVADO]`:
```
[PRIVADO] cliente 0: Hola, esto es privado
```

### 3. Comando usuarios

Al escribir `usuarios`, el servidor responde con la lista de IDs conectados:
```
conectados: 0,1,2
```

### 4. Desconexión limpia (logout)

Cuando un cliente escribe `exit`:
- Envía un mensaje de tipo `exit` al servidor
- El servidor elimina al cliente de la lista de conectados
- Cierra la conexión de forma ordenada
- Los demás clientes continúan funcionando sin problemas

### 5. Apagado ordenado del servidor (SIGINT)

El servidor puede apagarse ordenadamente con Ctrl+C:
- Captura la señal SIGINT
- Envía mensaje de cierre a todos los clientes conectados
- Los clientes reciben la notificación y cierran sus conexiones
- El servidor termina después de notificar a todos

## Cómo funciona

El servidor usa hilos (std::thread) para gestionar cada cliente de forma independiente. Cada conexión tiene su propio hilo que:
- Recibe mensajes del cliente
- Procesa el tipo de mensaje (texto, privado, usuarios, exit)
- Ejecuta la acción correspondiente
- Envía respuestas al cliente

Los tipos de mensajes se empaquetan usando las funciones `pack/unpack` de utils.h, siguiendo el patrón del código de ejemplo proporcionado.


