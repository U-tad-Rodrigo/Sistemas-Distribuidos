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
[SERVIDOR] Escuchando en puerto 5000...
[SERVIDOR] Esperando conexiones de clientes...
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
¡Conexión establecida!
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
¡Conexión establecida!
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
¡Conexión establecida!
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
[SERVIDOR] Escuchando en puerto 5000...
[SERVIDOR] Esperando conexiones de clientes...
============================================

[SERVIDOR] Cliente 1 conectado (socket: 4)
[SERVIDOR] Cliente 2 conectado (socket: 5)
[SERVIDOR] Cliente 3 conectado (socket: 6)
cliente 1: Soy el cliente 1, hola!
cliente 2: Soy el cliente 2, hola!
cliente 3: @1 Soy el cliente 3, mensaje privado a 1
cliente 1: usuarios
cliente 3: exit
cliente 2: exit
cliente 1: exit
```

**Terminal Cliente 1:**
```
$ ./client
========================================
       CLIENTE TCP INTERACTIVO
========================================
Conectando a 127.0.0.1:5000...
¡Conexión establecida!
========================================
Comandos disponibles:
  - Escribe un mensaje para enviarlo
  - 'usuarios' para ver clientes conectados
  - 'exit' para desconectar
  - '@ID mensaje' para mensaje privado
========================================

> cliente 2 se ha conectado
cliente 3 se ha conectado
Soy el cliente 1, hola!
Soy el cliente 1, hola!
> Servidor: mensaje recibido correctamente.
cliente 2: Soy el cliente 2, hola!
[PRIVADO] cliente 3: Soy el cliente 3, mensaje privado a 1
usuarios
usuarios
> conectados: 1,2,3
cliente 3 se ha desconectado
cliente 2 se ha desconectado
exit
exit
Cerrando conexión...
Servidor: desconectando...
Desconectado.
```

**Terminal Cliente 2:**
```
$ ./client
========================================
       CLIENTE TCP INTERACTIVO
========================================
Conectando a 127.0.0.1:5000...
¡Conexión establecida!
========================================
Comandos disponibles:
  - Escribe un mensaje para enviarlo
  - 'usuarios' para ver clientes conectados
  - 'exit' para desconectar
  - '@ID mensaje' para mensaje privado
========================================

> cliente 3 se ha conectado
cliente 1: Soy el cliente 1, hola!
Soy el cliente 2, hola!
Soy el cliente 2, hola!
> Servidor: mensaje recibido correctamente.
cliente 3 se ha desconectado
exit
exit
Cerrando conexión...
Servidor: desconectando...
Desconectado.
```

**Terminal Cliente 3:**
```
$ ./client
========================================
       CLIENTE TCP INTERACTIVO
========================================
Conectando a 127.0.0.1:5000...
¡Conexión establecida!
========================================
Comandos disponibles:
  - Escribe un mensaje para enviarlo
  - 'usuarios' para ver clientes conectados
  - 'exit' para desconectar
  - '@ID mensaje' para mensaje privado
========================================

> cliente 1: Soy el cliente 1, hola!
cliente 2: Soy el cliente 2, hola!
@1 Soy el cliente 3, mensaje privado a 1
@1 Soy el cliente 3, mensaje privado a 1
> Servidor: mensaje privado enviado a cliente 1
exit
exit
Cerrando conexión...
Servidor: desconectando...
Desconectado.

```

## Cómo funcionan las cosas

### Mensajes privados

Para mandar un mensaje a un cliente concreto escribes:
```
@ID mensaje
```

Ejemplo: `@2 Hola, esto es privado`

Cuando el servidor ve que el mensaje empieza con `@`, coge el número que viene después, busca al cliente con ese ID y le envía el mensaje solo a él. El que lo recibe lo ve con `[PRIVADO]` delante.

**Cómo lo hace el servidor:**
- Mira si el mensaje empieza por '@'
- Saca el ID que viene después
- Busca ese cliente en la lista
- Si existe, le manda el mensaje solo a él
- Si no existe, le dice al que lo envió que ese cliente no está


## Resumen

Todo funciona bien con varios clientes conectados. Los clientes pueden:
- Enviar mensajes que ven todos
- Mandar mensajes privados a uno solo
- Ver quién está conectado
- Desconectarse sin afectar a los demás

El servidor:
- Maneja varios clientes a la vez
- Mantiene la lista actualizada
- Avisa cuando alguien se conecta


