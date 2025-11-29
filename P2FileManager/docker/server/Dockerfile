# Autor: Rodrigo - Práctica 2 Sistemas Distribuidos
# Dockerfile para serverFileManager

FROM ubuntu:20.04

WORKDIR /app

# Copiar el ejecutable del server al contenedor
COPY serverFileManager /app/serverFileManager

# Dar permisos de ejecución
RUN chmod +x /app/serverFileManager

# Crear directorio para los archivos compartidos
RUN mkdir -p /FileManagerDir

# Exponer el puerto del server
EXPOSE 32001

# No se define CMD aquí porque los argumentos se pasarán desde Kubernetes

