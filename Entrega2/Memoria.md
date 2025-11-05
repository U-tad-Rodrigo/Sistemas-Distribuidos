# Memoria - Chat Cliente-Servidor

## Qué hace el programa

Es un chat básico donde un servidor puede tener varios clientes conectados a la vez. Los clientes se envían mensajes entre ellos y el servidor los gestiona.

### Lo que tiene

- Varios clientes pueden conectarse al mismo tiempo (usa hilos para cada uno)
- Los mensajes se reenvían a todos los clientes
- Puedes enviar mensajes privados con `@ID mensaje`
- El comando `usuarios` te dice quién está conectado


## Prueba con 3 clientes

Para probarlo abrí 4 terminales: una para el servidor y tres para los clientes.

### Paso 1: Arrancar el servidor

Primero arranco el servidor y lo dejo esperando:

```
$ ./server
============================================
  SERVIDOR TCP MULTI-CLIENTE CON BROADCAST
============================================
Puerto: 5000
============================================
Servidor abriendo puerto...
Puerto abierto, esperando conexiones...
============================================

```

### Paso 2: Conectar los 3 clientes

Ahora abro 3 terminales más y ejecuto el cliente en cada una:

**Cliente 1:**
```
$ ./client
========================================
       CLIENTE TCP INTERACTIVO
========================================
Conectando a 127.0.0.1:5000...
Conexion establecida!
========================================
Comandos disponibles:
  - Escribe un mensaje para enviarlo
  - 'usuarios' para ver clientes conectados
  - 'exit' para desconectar
  - '@ID mensaje' para mensaje privado
========================================

>
```

**Cliente 2:**
```
$ ./client
========================================
       CLIENTE TCP INTERACTIVO
========================================
Conectando a 127.0.0.1:5000...
Conexion establecida!
========================================
Comandos disponibles:
  - Escribe un mensaje para enviarlo
  - 'usuarios' para ver clientes conectados
  - 'exit' para desconectar
  - '@ID mensaje' para mensaje privado
========================================

> 
```

**Cliente 3:**
```
$ ./client
========================================
       CLIENTE TCP INTERACTIVO
========================================
Conectando a 127.0.0.1:5000...
Conexion establecida!
========================================
Comandos disponibles:
  - Escribe un mensaje para enviarlo
  - 'usuarios' para ver clientes conectados
  - 'exit' para desconectar
  - '@ID mensaje' para mensaje privado
========================================

> 
```

### Paso 3: Conversación completa

**Terminal del servidor:**
```
$ ./server
============================================
  SERVIDOR TCP MULTI-CLIENTE CON BROADCAST
============================================
Puerto: 5000
============================================
Servidor abriendo puerto...
Puerto abierto, esperando conexiones...
============================================

[SERVIDOR] Cliente ID: 0 conectado
[SERVIDOR] Cliente ID: 1 conectado
[SERVIDOR] Cliente ID: 2 conectado
cliente 0: Soy el cliente 1, hola!
cliente 1: Soy el cliente 2, hola!
cliente 2: Hola desde el cliente 3
[SERVIDOR] Cliente 2 desconectado
[SERVIDOR] Cliente 1 desconectado
[SERVIDOR] Cliente 0 desconectado
```

**Terminal Cliente 1:**
```
$ ./client
========================================
       CLIENTE TCP INTERACTIVO
========================================
Conectando a 127.0.0.1:5000...
Conexion establecida!
========================================
Comandos disponibles:
  - Escribe un mensaje para enviarlo
  - 'usuarios' para ver clientes conectados
  - 'exit' para desconectar
  - '@ID mensaje' para mensaje privado
========================================

> Soy el cliente 1, hola!
Servidor: mensaje recibido correctamente.
> cliente 1: Soy el cliente 2, hola!
cliente 2: Hola desde el cliente 3
usuarios
conectados: 0,1,2
> @2 Este es un mensaje privado para ti
Servidor: mensaje privado enviado a cliente 2
> exit
Cerrando conexion...
Desconectado.
```

**Terminal Cliente 2:**
```
$ ./client
========================================
       CLIENTE TCP INTERACTIVO
========================================
Conectando a 127.0.0.1:5000...
Conexion establecida!
========================================
Comandos disponibles:
  - Escribe un mensaje para enviarlo
  - 'usuarios' para ver clientes conectados
  - 'exit' para desconectar
  - '@ID mensaje' para mensaje privado
========================================

> cliente 0: Soy el cliente 1, hola!
Soy el cliente 2, hola!
Servidor: mensaje recibido correctamente.
> cliente 2: Hola desde el cliente 3
[PRIVADO] cliente 0: Este es un mensaje privado para ti
exit
Cerrando conexion...
Desconectado.
```

**Terminal Cliente 3:**
```
$ ./client
========================================
       CLIENTE TCP INTERACTIVO
========================================
Conectando a 127.0.0.1:5000...
Conexion establecida!
========================================
Comandos disponibles:
  - Escribe un mensaje para enviarlo
  - 'usuarios' para ver clientes conectados
  - 'exit' para desconectar
  - '@ID mensaje' para mensaje privado
========================================

> cliente 0: Soy el cliente 1, hola!
cliente 1: Soy el cliente 2, hola!
Hola desde el cliente 3
Servidor: mensaje recibido correctamente.
> exit
Cerrando conexion...
Desconectado.

```

## Cómo funcionan las cosas

### Mensajes privados

Para mandar un mensaje a un cliente concreto escribes:
```
@ID mensaje
```

Ejemplo: `@2 Hola, esto es privado`

El servidor busca al cliente con ese ID y le envía el mensaje solo a él. El que lo recibe lo ve con `[PRIVADO]` delante.

### Comando usuarios

Si escribes `usuarios` el servidor te manda la lista de IDs conectados en ese momento.

### Desconexión

Cuando escribes `exit`, el cliente cierra la conexión y el servidor lo quita de la lista. Los demás siguen funcionando normal.


## Resumen

El programa funciona con varios clientes a la vez. Cada cliente puede mandar mensajes a todos, mensajes privados a uno solo, ver quién está conectado y desconectarse cuando quiera. El servidor gestiona todo usando hilos.


