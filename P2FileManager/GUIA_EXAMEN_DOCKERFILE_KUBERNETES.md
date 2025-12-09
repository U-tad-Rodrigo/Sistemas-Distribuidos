# 📚 GUÍA DEFINITIVA: DOCKERFILE Y KUBERNETES YAML
## Para Examen de Sistemas Distribuidos

**Autor:** Rodrigo  
**Fecha:** Diciembre 2025  
**Objetivo:** Aprender a escribir a papel Dockerfiles y YAMLs de Kubernetes

---

## 📋 ÍNDICE

1. [Dockerfile - Conceptos Básicos](#1-dockerfile---conceptos-básicos)
2. [Dockerfile - Estructura Completa](#2-dockerfile---estructura-completa)
3. [Kubernetes YAML - Conceptos Básicos](#3-kubernetes-yaml---conceptos-básicos)
4. [Deployment - Estructura](#4-deployment---estructura)
5. [Service - Estructura](#5-service---estructura)
6. [PersistentVolume y PVC](#6-persistentvolume-y-pvc)
7. [Casos Prácticos: 3 Escenarios](#7-casos-prácticos-3-escenarios)
8. [Checklist para el Examen](#8-checklist-para-el-examen)

---

## 1. DOCKERFILE - CONCEPTOS BÁSICOS

### ¿Qué es un Dockerfile?
Un archivo que contiene instrucciones para construir una imagen Docker. Es como una receta para crear un contenedor.

### Instrucciones Fundamentales (Las que SÍ o SÍ debes saber)

| Instrucción | Propósito | Ejemplo |
|-------------|-----------|---------|
| `FROM` | Imagen base (siempre la primera) | `FROM ubuntu:22.04` |
| `RUN` | Ejecuta comandos al construir la imagen | `RUN apt-get update` |
| `COPY` | Copia archivos del host a la imagen | `COPY app.py /app/` |
| `WORKDIR` | Establece directorio de trabajo | `WORKDIR /app` |
| `EXPOSE` | Documenta qué puerto usa el contenedor | `EXPOSE 8080` |
| `CMD` | Comando por defecto al ejecutar | `CMD ["/app"]` |
| `ENV` | Variables de entorno | `ENV PORT=8080` |

### Diferencias Importantes

#### `RUN` vs `CMD`
- **RUN**: Se ejecuta al **CONSTRUIR** la imagen (instalar paquetes, crear directorios)
- **CMD**: Se ejecuta al **INICIAR** el contenedor (arrancar la aplicación)

```dockerfile
RUN apt-get install -y curl    # Durante build
CMD ["/mi-aplicacion"]          # Al hacer docker run
```

#### `COPY` vs `ADD`
- **COPY**: Solo copia archivos (recomendado)
- **ADD**: Copia y puede descomprimir (menos usado)

```dockerfile
COPY app.py /app/         # Preferido
ADD archive.tar.gz /app/  # Solo si necesitas descomprimir
```

---

## 2. DOCKERFILE - ESTRUCTURA COMPLETA

### Template Básico (Para Memorizar)

```dockerfile
# 1. IMAGEN BASE
FROM ubuntu:22.04

# 2. INSTALACIÓN DE DEPENDENCIAS
RUN apt-get update && \
    apt-get install -y paquete1 paquete2 && \
    rm -rf /var/lib/apt/lists/*

# 3. COPIAR ARCHIVOS
COPY mi-binario /ruta/destino/mi-binario

# 4. PERMISOS Y CONFIGURACIÓN
RUN chmod +x /ruta/destino/mi-binario
RUN mkdir -p /datos

# 5. EXPONER PUERTOS
EXPOSE 8080

# 6. COMANDO DE INICIO
CMD ["/ruta/destino/mi-binario"]
```

### Ejemplo Real: Broker FileManager

```dockerfile
# Autor: Rodrigo
# Dockerfile para brokerFileManager

# 1. Imagen base Ubuntu
FROM ubuntu:22.04

# 2. Instalar herramientas necesarias
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        ca-certificates \
        net-tools \
    && rm -rf /var/lib/apt/lists/*

# 3. Copiar el binario
COPY brokerFileManager /brokerFileManager

# 4. Dar permisos de ejecución
RUN chmod +x /brokerFileManager

# 5. Exponer puerto
EXPOSE 32002

# 6. Comando por defecto
CMD ["/brokerFileManager"]
```

### Ejemplo Real: Server FileManager

```dockerfile
# Autor: Rodrigo
# Dockerfile para serverFileManager

FROM ubuntu:22.04

# Instalar curl (para obtener IP), ca-certificates, net-tools
RUN apt-get update && \
    apt-get install -y \
        curl \
        ca-certificates \
        net-tools \
    && rm -rf /var/lib/apt/lists/*

# Copiar el ejecutable
COPY ./serverFileManager /serverFileManager

# Crear directorio para archivos
RUN mkdir -p /FileManagerDir && chmod -R 777 /FileManagerDir

# Dar permisos
RUN chmod +x /serverFileManager

# Exponer puerto
EXPOSE 32001

# Comando por defecto
CMD ["/serverFileManager"]
```

### Buenas Prácticas (Importante para el Examen)

1. **Combinar RUN**: Usar `&&` y `\` para reducir capas
   ```dockerfile
   # ❌ MAL (3 capas)
   RUN apt-get update
   RUN apt-get install curl
   RUN apt-get install git
   
   # ✅ BIEN (1 capa)
   RUN apt-get update && \
       apt-get install -y curl git
   ```

2. **Limpiar cache**: `rm -rf /var/lib/apt/lists/*`
3. **Usar --no-install-recommends**: Instalar solo lo necesario
4. **Permisos explícitos**: `chmod +x` para binarios

---

## 3. KUBERNETES YAML - CONCEPTOS BÁSICOS

### ¿Qué es un YAML de Kubernetes?
Archivo de configuración que define cómo desplegar aplicaciones en Kubernetes.

### Estructura Básica de TODO YAML

```yaml
apiVersion: apps/v1        # Versión de la API
kind: Deployment           # Tipo de recurso
metadata:                  # Metadatos
  name: mi-app
  namespace: default
spec:                      # Especificación
  replicas: 2
  # ... resto de configuración
```

### 4 Campos Obligatorios en TODO YAML

1. **apiVersion**: Versión de la API de Kubernetes
2. **kind**: Tipo de recurso (Deployment, Service, Pod, etc.)
3. **metadata**: Información del recurso (nombre, labels)
4. **spec**: Especificación del recurso (configuración real)

### Versiones de API Comunes

| Tipo de Recurso | apiVersion |
|-----------------|------------|
| Deployment | `apps/v1` |
| Service | `v1` |
| Pod | `v1` |
| PersistentVolume | `v1` |
| PersistentVolumeClaim | `v1` |

---

## 4. DEPLOYMENT - ESTRUCTURA

### ¿Qué es un Deployment?
Gestiona un conjunto de Pods réplicas. Asegura que X número de Pods estén siempre corriendo.

### Template para Memorizar

```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: nombre-deployment
  namespace: default
spec:
  replicas: 2                    # Número de pods
  selector:                      # Cómo encuentra sus pods
    matchLabels:
      app: mi-label
  template:                      # Template del Pod
    metadata:
      labels:
        app: mi-label            # Debe coincidir con selector
    spec:
      containers:
      - name: nombre-contenedor
        image: usuario/imagen:tag
        ports:
        - containerPort: 8080
        env:                     # Variables de entorno (opcional)
        - name: VAR_NAME
          value: "valor"
        volumeMounts:            # Volúmenes (opcional)
        - name: vol-name
          mountPath: /ruta
      volumes:                   # Definición de volúmenes
      - name: vol-name
        emptyDir: {}
```

### Partes Críticas para Entender

#### 1. Selector y Labels (FUNDAMENTAL)
```yaml
spec:
  selector:
    matchLabels:
      app: mi-app        # ← Debe coincidir ↓
  template:
    metadata:
      labels:
        app: mi-app      # ← con este label
```
**¿Por qué?** El Deployment encuentra sus Pods por labels.

#### 2. Réplicas
```yaml
replicas: 2    # Kubernetes mantendrá 2 pods corriendo
```

#### 3. Container Spec
```yaml
containers:
- name: mi-contenedor           # Nombre del contenedor
  image: docker-user/app:v1     # Imagen de Docker Hub
  imagePullPolicy: Always       # Cuándo descargar imagen
  ports:
  - containerPort: 8080          # Puerto que expone el contenedor
```

### Caso 1: Deployment BÁSICO (Sin Volúmenes)

```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: broker
spec:
  replicas: 1
  selector:
    matchLabels:
      app: broker
  template:
    metadata:
      labels:
        app: broker
    spec:
      containers:
      - name: broker
        image: skitemplar/broker-filemanager:v1
        ports:
        - containerPort: 32002
```

**Características:**
- 1 réplica (broker único)
- Sin volúmenes (no necesita persistencia)
- Puerto 32002

### Caso 2: Deployment con VARIABLES DE ENTORNO

```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: server
spec:
  replicas: 2
  selector:
    matchLabels:
      app: server
  template:
    metadata:
      labels:
        app: server
    spec:
      containers:
      - name: server
        image: skitemplar/server-filemanager:v1
        command: ["/serverFileManager"]
        args:
        - "$(BROKER_SERVICE_SERVICE_HOST)"    # Variable auto-creada por K8s
        - "32002"
        - "$(MY_NODE_IP)"                      # Variable custom
        - "32001"
        env:
        - name: MY_NODE_IP
          valueFrom:
            fieldRef:
              fieldPath: status.hostIP         # IP del nodo
        ports:
        - containerPort: 32001
```

**Características:**
- `command`: Sobreescribe CMD del Dockerfile
- `args`: Argumentos para el comando
- `env`: Define variables de entorno
- `fieldRef`: Obtiene info del nodo/pod

### Caso 3: Deployment con SCRIPT INLINE

```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: server-filemanager-deployment
spec:
  replicas: 2
  selector:
    matchLabels:
      app: serverfilemanager
  template:
    metadata:
      labels:
        app: serverfilemanager
    spec:
      containers:
      - name: server-container
        image: skitemplar/server-filemanager:v1
        command: ["/bin/sh", "-c"]    # Ejecuta shell script
        args:
        - |                           # Script multi-línea
          echo "Iniciando servidor..."
          
          # Instalar curl dinámicamente
          if ! command -v curl &> /dev/null; then
            apt-get update -qq && apt-get install -y -qq curl
          fi
          
          # Obtener IP pública del nodo
          export PUBLIC_IP=$(curl -s https://checkip.amazonaws.com)
          echo "IP Publica: $PUBLIC_IP"
          
          # Resolver IP del broker
          BROKER_IP=$(getent hosts broker-service | awk '{ print $1 }')
          echo "Broker IP: $BROKER_IP"
          
          # Ejecutar aplicación
          exec /serverFileManager $BROKER_IP 32002 $PUBLIC_IP 32001
        ports:
        - containerPort: 32001
```

**Características:**
- Script bash completo dentro del YAML
- Instalación dinámica de herramientas
- Resolución de IPs automática
- `exec`: Reemplaza el proceso shell por la app

### Caso 4: Deployment con HOSTPATH (Volumen Local)

```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: server-filemanager-deployment
spec:
  replicas: 2
  selector:
    matchLabels:
      app: serverfilemanager
  template:
    metadata:
      labels:
        app: serverfilemanager
    spec:
      containers:
      - name: server-container
        image: skitemplar/server-filemanager:v1
        ports:
        - containerPort: 32001
        volumeMounts:                    # Montaje en el contenedor
        - name: shared-dir
          mountPath: /FileManagerDir     # Ruta dentro del contenedor
      volumes:                           # Definición del volumen
      - name: shared-dir
        hostPath:                        # Volumen del nodo
          path: /srv/filemanager/hostpath   # Ruta en el nodo
          type: Directory
```

**Características:**
- `volumeMounts`: Dónde montar EN el contenedor
- `volumes`: QUÉ montar (origen)
- `hostPath`: Usa directorio del nodo
- **Limitación**: Pods en el mismo nodo comparten, en diferentes nodos NO

### Caso 5: Deployment con NFS (Volumen Compartido)

```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: server-filemanager-deployment
spec:
  replicas: 2
  selector:
    matchLabels:
      app: serverfilemanager
  template:
    metadata:
      labels:
        app: serverfilemanager
    spec:
      containers:
      - name: server-container
        image: skitemplar/server-filemanager:v1
        ports:
        - containerPort: 32001
        volumeMounts:
        - name: directorio-filemanager-nfs
          mountPath: /FileManagerDir
      volumes:
      - name: directorio-filemanager-nfs
        persistentVolumeClaim:           # Usa un PVC
          claimName: nfs-pvc             # Nombre del PVC
```

**Características:**
- Usa `persistentVolumeClaim` en vez de `hostPath`
- Requiere crear PV y PVC separados
- **Ventaja**: Todos los nodos acceden al mismo almacenamiento

---

## 5. SERVICE - ESTRUCTURA

### ¿Qué es un Service?
Expone un conjunto de Pods a la red. Proporciona una IP/DNS estable para acceder a los Pods.

### Template para Memorizar

```yaml
apiVersion: v1
kind: Service
metadata:
  name: nombre-service
spec:
  type: NodePort              # Tipo de servicio
  selector:                   # Selecciona pods
    app: mi-label             # Label de los pods
  ports:
  - port: 8080                # Puerto del servicio
    targetPort: 8080          # Puerto del contenedor
    nodePort: 30080           # Puerto expuesto externamente (30000-32767)
    protocol: TCP
```

### Tipos de Service

| Tipo | Descripción | Cuándo Usar |
|------|-------------|-------------|
| **ClusterIP** | Solo accesible dentro del clúster | Comunicación interna (DB, cache) |
| **NodePort** | Accesible desde fuera en todos los nodos | Desarrollo, testing |
| **LoadBalancer** | Crea balanceador de carga externo | Producción en cloud |

### 3 Puertos Importantes

```yaml
ports:
- port: 32002         # Puerto del SERVICE (dentro del clúster)
  targetPort: 32002   # Puerto del CONTENEDOR (Pod)
  nodePort: 32002     # Puerto del NODO (acceso externo)
```

**Ejemplo:**
- Dentro del clúster: `broker-service:32002`
- Desde fuera: `<IP-NODO>:32002`

### Service Completo - Broker

```yaml
apiVersion: v1
kind: Service
metadata:
  name: broker-service
spec:
  type: NodePort
  selector:
    app: broker           # Selecciona pods con label app=broker
  ports:
  - port: 32002           # Puerto interno
    targetPort: 32002     # Puerto del contenedor
    nodePort: 32002       # Puerto externo
    protocol: TCP
```

### Service Completo - Server

```yaml
apiVersion: v1
kind: Service
metadata:
  name: server-service
spec:
  type: NodePort
  selector:
    app: serverfilemanager
  ports:
  - port: 32001
    targetPort: 32001
    nodePort: 30010              # Puede ser diferente
    protocol: TCP
  externalTrafficPolicy: Cluster  # Opcional: routing policy
```

### Relación Service ↔ Deployment

```yaml
# DEPLOYMENT
selector:
  matchLabels:
    app: broker      # ← Label del pod

---

# SERVICE
selector:
  app: broker        # ← Mismo label
```

**Regla de Oro:** El `selector` del Service debe coincidir con los `labels` del Pod.

---

## 6. PERSISTENTVOLUME Y PVC

### ¿Para Qué Sirven?

- **PersistentVolume (PV)**: Almacenamiento físico (NFS, disco, etc.)
- **PersistentVolumeClaim (PVC)**: "Solicitud" de almacenamiento
- **Deployment**: "Usa" el PVC

```
[PV] ← reclama ← [PVC] ← monta ← [Deployment]
```

### PersistentVolume con NFS

```yaml
apiVersion: v1
kind: PersistentVolume
metadata:
  name: filemanager-nfs-pv
spec:
  storageClassName: storage-nfs     # Nombre de la clase
  capacity:
    storage: 10Gi                   # Tamaño del volumen
  accessModes:
    - ReadWriteMany                 # Múltiples nodos pueden escribir
  nfs:
    server: 172.31.64.246           # IP del servidor NFS
    path: /export/k8s-files-filemanager  # Ruta compartida
```

### PersistentVolumeClaim

```yaml
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: nfs-pvc
spec:
  storageClassName: storage-nfs     # Debe coincidir con PV
  accessModes:
    - ReadWriteMany                 # Debe coincidir con PV
  resources:
    requests:
      storage: 10Gi                 # Cuánto se solicita
```

### Access Modes (Importante)

| Modo | Abreviatura | Significado |
|------|-------------|-------------|
| ReadWriteOnce | RWO | 1 nodo puede montar (lectura/escritura) |
| ReadOnlyMany | ROX | Múltiples nodos (solo lectura) |
| ReadWriteMany | RWX | Múltiples nodos (lectura/escritura) |

**Para NFS:** Usar `ReadWriteMany` (RWX)  
**Para hostPath:** Usar `ReadWriteOnce` (RWO)

---

## 7. CASOS PRÁCTICOS: 3 ESCENARIOS

### ESCENARIO 1: BÁSICO (Sin Almacenamiento Compartido)

**Arquitectura:**
- Broker: 1 réplica
- Server: 2 réplicas
- Sin volúmenes persistentes
- Comunicación via Service

**Archivos:**

#### `broker.yaml`
```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: broker
spec:
  replicas: 1
  selector:
    matchLabels:
      app: broker
  template:
    metadata:
      labels:
        app: broker
    spec:
      containers:
      - name: broker
        image: usuario/broker:v1
        ports:
        - containerPort: 32002

---

apiVersion: v1
kind: Service
metadata:
  name: broker-service
spec:
  type: NodePort
  selector:
    app: broker
  ports:
  - port: 32002
    targetPort: 32002
    nodePort: 32002
```

#### `server.yaml`
```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: server
spec:
  replicas: 2
  selector:
    matchLabels:
      app: server
  template:
    metadata:
      labels:
        app: server
    spec:
      containers:
      - name: server
        image: usuario/server:v1
        command: ["/serverFileManager"]
        args:
        - "$(BROKER_SERVICE_SERVICE_HOST)"
        - "32002"
        - "$(MY_NODE_IP)"
        - "32001"
        env:
        - name: MY_NODE_IP
          valueFrom:
            fieldRef:
              fieldPath: status.hostIP
        ports:
        - containerPort: 32001

---

apiVersion: v1
kind: Service
metadata:
  name: server-service
spec:
  type: NodePort
  selector:
    app: server
  ports:
  - port: 32001
    targetPort: 32001
    nodePort: 30010
```

**Características:**
- ✅ Fácil de escribir
- ✅ Sin dependencias externas
- ❌ Cada servidor tiene su propio almacenamiento (no compartido)

---

### ESCENARIO 2: AVANZADO con HOSTPATH

**Arquitectura:**
- Almacenamiento local del nodo
- Pods en el MISMO nodo comparten archivos
- Pods en DIFERENTES nodos NO comparten

**Archivos:**

#### `server.yaml` (con hostPath)
```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: server-filemanager-deployment
spec:
  replicas: 2
  selector:
    matchLabels:
      app: serverfilemanager
  template:
    metadata:
      labels:
        app: serverfilemanager
    spec:
      containers:
      - name: server-container
        image: usuario/server:v1
        command: ["/bin/sh", "-c"]
        args:
        - |
          # Script de inicialización
          PUBLIC_IP=$(curl -s https://checkip.amazonaws.com)
          BROKER_IP=$(getent hosts broker-service | awk '{ print $1 }')
          exec /serverFileManager $BROKER_IP 32002 $PUBLIC_IP 32001
        ports:
        - containerPort: 32001
        volumeMounts:
        - name: shared-dir
          mountPath: /FileManagerDir
      volumes:
      - name: shared-dir
        hostPath:
          path: /srv/filemanager/hostpath
          type: Directory

---

apiVersion: v1
kind: Service
metadata:
  name: server-service
spec:
  type: NodePort
  selector:
    app: serverfilemanager
  ports:
  - port: 32001
    targetPort: 32001
    nodePort: 30010
```

**Características:**
- ✅ Compartir entre pods del mismo nodo
- ✅ No necesita NFS
- ❌ No funciona entre diferentes nodos

---

### ESCENARIO 3: AVANZADO con NFS

**Arquitectura:**
- Almacenamiento NFS centralizado
- TODOS los pods comparten archivos
- Funciona en múltiples nodos

**Archivos:**

#### `nfs-storage.yaml`
```yaml
apiVersion: v1
kind: PersistentVolume
metadata:
  name: filemanager-nfs-pv
spec:
  storageClassName: storage-nfs
  capacity:
    storage: 10Gi
  accessModes:
    - ReadWriteMany
  nfs:
    server: 172.31.64.246
    path: /export/k8s-files-filemanager

---

apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: nfs-pvc
spec:
  storageClassName: storage-nfs
  accessModes:
    - ReadWriteMany
  resources:
    requests:
      storage: 10Gi
```

#### `server.yaml` (con NFS)
```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: server-filemanager-deployment
spec:
  replicas: 2
  selector:
    matchLabels:
      app: serverfilemanager
  template:
    metadata:
      labels:
        app: serverfilemanager
    spec:
      containers:
      - name: server-container
        image: usuario/server:v1
        command: ["/bin/sh", "-c"]
        args:
        - |
          PUBLIC_IP=$(curl -s https://checkip.amazonaws.com)
          BROKER_IP=$(getent hosts broker-service | awk '{ print $1 }')
          exec /serverFileManager $BROKER_IP 32002 $PUBLIC_IP 32001
        ports:
        - containerPort: 32001
        volumeMounts:
        - name: directorio-filemanager-nfs
          mountPath: /FileManagerDir
      volumes:
      - name: directorio-filemanager-nfs
        persistentVolumeClaim:
          claimName: nfs-pvc

---

apiVersion: v1
kind: Service
metadata:
  name: server-service
spec:
  type: NodePort
  selector:
    app: serverfilemanager
  ports:
  - port: 32001
    targetPort: 32001
    nodePort: 32001
```

**Características:**
- ✅ Almacenamiento compartido entre todos los nodos
- ✅ Persistencia de datos
- ❌ Requiere servidor NFS configurado

---

## 8. CHECKLIST PARA EL EXAMEN

### ✅ Dockerfile - Checklist

Cuando escribas un Dockerfile, asegúrate de:

1. [ ] `FROM` como primera línea (imagen base)
2. [ ] `RUN apt-get update && apt-get install...` (dependencias)
3. [ ] `COPY` tu aplicación al contenedor
4. [ ] `RUN chmod +x` para dar permisos al binario
5. [ ] `RUN mkdir` si necesitas crear directorios
6. [ ] `EXPOSE` el puerto que usa tu app
7. [ ] `CMD` para arrancar la aplicación

**Orden lógico:**
```
FROM → RUN (instalar) → COPY → RUN (configurar) → EXPOSE → CMD
```

---

### ✅ Deployment - Checklist

1. [ ] `apiVersion: apps/v1`
2. [ ] `kind: Deployment`
3. [ ] `metadata` con `name`
4. [ ] `spec.replicas` (número de pods)
5. [ ] `spec.selector.matchLabels` con un label
6. [ ] `template.metadata.labels` (mismo label que selector)
7. [ ] `template.spec.containers`:
   - [ ] `name`
   - [ ] `image`
   - [ ] `ports` con `containerPort`
8. [ ] Si necesitas volúmenes:
   - [ ] `volumeMounts` en el contenedor
   - [ ] `volumes` en el spec del pod

---

### ✅ Service - Checklist

1. [ ] `apiVersion: v1`
2. [ ] `kind: Service`
3. [ ] `metadata` con `name`
4. [ ] `spec.type: NodePort` (o ClusterIP)
5. [ ] `spec.selector` con el label del Deployment
6. [ ] `spec.ports`:
   - [ ] `port` (puerto del servicio)
   - [ ] `targetPort` (puerto del contenedor)
   - [ ] `nodePort` (si es NodePort)

---

### ✅ PV y PVC - Checklist (Solo si hay NFS)

**PersistentVolume:**
1. [ ] `apiVersion: v1`
2. [ ] `kind: PersistentVolume`
3. [ ] `spec.storageClassName`
4. [ ] `spec.capacity.storage`
5. [ ] `spec.accessModes` (ReadWriteMany para NFS)
6. [ ] `spec.nfs.server` (IP del NFS)
7. [ ] `spec.nfs.path` (ruta en NFS)

**PersistentVolumeClaim:**
1. [ ] `apiVersion: v1`
2. [ ] `kind: PersistentVolumeClaim`
3. [ ] `spec.storageClassName` (mismo que PV)
4. [ ] `spec.accessModes` (mismo que PV)
5. [ ] `spec.resources.requests.storage`

---

## 9. ERRORES COMUNES (Lo que NO Hacer)

### ❌ Error 1: Labels no coinciden
```yaml
# MAL
selector:
  matchLabels:
    app: broker
template:
  metadata:
    labels:
      app: servidor  # ← NO COINCIDE
```

### ❌ Error 2: Olvidar imagePullPolicy
```yaml
# Si la imagen es la misma versión, puede usar caché
image: usuario/app:v1
imagePullPolicy: Always  # Forzar descarga
```

### ❌ Error 3: Puerto incorrecto en Service
```yaml
# El targetPort debe coincidir con containerPort del Deployment
ports:
- containerPort: 32001  # En Deployment

# En Service:
ports:
- targetPort: 32001     # Debe coincidir
```

### ❌ Error 4: storageClassName diferente
```yaml
# PV
storageClassName: storage-nfs

# PVC
storageClassName: nfs-storage  # ← NO COINCIDE
```

### ❌ Error 5: Olvidar el separador ---
```yaml
# Para múltiples recursos en un archivo
apiVersion: apps/v1
kind: Deployment
...

---  # ← NECESARIO

apiVersion: v1
kind: Service
...
```

---

## 10. TIPS PARA ESCRIBIR A PAPEL

### Memoriza estas Estructuras

**1. Dockerfile Mínimo:**
```
FROM imagen:tag
RUN apt-get update && apt-get install -y paquetes
COPY app /app
RUN chmod +x /app
EXPOSE puerto
CMD ["/app"]
```

**2. Deployment Mínimo:**
```
apiVersion: apps/v1
kind: Deployment
metadata:
  name: nombre
spec:
  replicas: X
  selector:
    matchLabels:
      app: label
  template:
    metadata:
      labels:
        app: label
    spec:
      containers:
      - name: container
        image: imagen:tag
        ports:
        - containerPort: XXXX
```

**3. Service Mínimo:**
```
apiVersion: v1
kind: Service
metadata:
  name: nombre-service
spec:
  type: NodePort
  selector:
    app: label
  ports:
  - port: XXXX
    targetPort: XXXX
    nodePort: XXXXX
```

### Técnica de Memorización

1. **Dockerfile**: FROM → RUN → COPY → RUN → EXPOSE → CMD
2. **Deployment**: api → kind → metadata → spec (replicas → selector → template)
3. **Service**: api → kind → metadata → spec (type → selector → ports)

### En el Examen

1. Escribe primero la estructura (apiVersion, kind, metadata, spec)
2. Luego rellena los detalles
3. Verifica que los labels coincidan (selector ↔ labels)
4. Verifica que los puertos coincidan (containerPort ↔ targetPort)

---

## 11. COMPARACIÓN RÁPIDA

### Casos de Uso

| Necesidad | Solución |
|-----------|----------|
| App sin estado (stateless) | Deployment básico sin volúmenes |
| Almacenamiento en 1 nodo | Deployment + hostPath |
| Almacenamiento compartido multi-nodo | Deployment + NFS (PV + PVC) |
| Exponer app internamente | Service tipo ClusterIP |
| Exponer app externamente | Service tipo NodePort |

### Volúmenes

| Tipo | Ventaja | Desventaja |
|------|---------|------------|
| Sin volumen | Simple | Datos se pierden |
| emptyDir | Temporal, fácil | Se borra al eliminar pod |
| hostPath | No necesita NFS | Solo funciona en 1 nodo |
| NFS (PV+PVC) | Compartido multi-nodo | Requiere servidor NFS |

---

## 12. EJERCICIO FINAL (Practica Esto)

### Escribe de memoria (sin mirar):

**1. Dockerfile para una app Python:**
- Imagen base: `python:3.9`
- Instalar dependencias: `RUN pip install flask`
- Copiar app: `COPY app.py /app/app.py`
- Puerto: 5000
- Comando: `CMD ["python", "/app/app.py"]`

**2. Deployment para la app:**
- Nombre: `flask-app`
- 3 réplicas
- Label: `app: flask`
- Imagen: `usuario/flask:v1`
- Puerto: 5000

**3. Service para la app:**
- Nombre: `flask-service`
- NodePort
- Puerto: 5000
- NodePort: 30500

---

## 13. RESUMEN ULTRA-RÁPIDO

### Dockerfile
```
FROM base
RUN instalar
COPY archivo destino
RUN configurar
EXPOSE puerto
CMD ["comando"]
```

### Deployment
```
apiVersion: apps/v1
kind: Deployment
spec:
  replicas: N
  selector: {label}
  template:
    labels: {mismo label}
    containers:
    - image: ...
      ports: [containerPort]
      volumeMounts: [opcional]
    volumes: [opcional]
```

### Service
```
apiVersion: v1
kind: Service
spec:
  type: NodePort
  selector: {label del deployment}
  ports:
  - port: X
    targetPort: X
    nodePort: XXXXX
```

### PV + PVC (NFS)
```
PV:
  storageClassName: X
  capacity: 10Gi
  accessModes: [ReadWriteMany]
  nfs: {server, path}

PVC:
  storageClassName: X (mismo)
  accessModes: [ReadWriteMany]
  storage: 10Gi
```

---

## 📝 NOTAS FINALES

### Lo Más Importante

1. **Labels**: Deben coincidir entre selector y template
2. **Puertos**: containerPort ↔ targetPort
3. **StorageClassName**: Debe ser igual en PV y PVC
4. **Indentación**: YAML es muy sensible a espacios (usa 2 espacios)

### Comando para Verificar Sintaxis (Opcional)

Si tienes tiempo en el examen para repasar mentalmente:
- ¿FROM está primero? ✓
- ¿Los labels coinciden? ✓
- ¿Los puertos coinciden? ✓
- ¿El storageClassName es igual en PV y PVC? ✓

---

## 🎯 ESTRATEGIA DE ESTUDIO

### Día 1-2: Dockerfile
- Memoriza el orden: FROM → RUN → COPY → RUN → EXPOSE → CMD
- Practica escribir 5 Dockerfiles diferentes de memoria

### Día 3-4: Deployment
- Memoriza la estructura base
- Practica los 3 casos: básico, hostPath, NFS

### Día 5-6: Service
- Memoriza los 3 puertos: port, targetPort, nodePort
- Practica escribir Services de memoria

### Día 7: Repaso General
- Escribe los 3 escenarios completos de memoria
- Verifica con esta guía

---

## ✅ AUTOEVALUACIÓN

¿Puedes escribir de memoria sin mirar?

- [ ] Un Dockerfile con 6 instrucciones
- [ ] Un Deployment básico (sin volúmenes)
- [ ] Un Deployment con hostPath
- [ ] Un Deployment con NFS
- [ ] Un Service tipo NodePort
- [ ] Un PersistentVolume con NFS
- [ ] Un PersistentVolumeClaim

Si respondes SÍ a todos, ¡estás listo para el examen! 🎉

---

**¡MUCHA SUERTE EN EL EXAMEN!** 🚀

Recuerda: No se trata de sintaxis perfecta, sino de entender la ESTRUCTURA y saber QUÉ poner en cada campo.

