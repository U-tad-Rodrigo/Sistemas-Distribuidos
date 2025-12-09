# Autor: Rodrigo - Práctica 2 Sistemas Distribuidos
# Dockerfile para serverFileManager

FROM ubuntu:22.04

# Instalar herramientas necesarias: curl para obtener IP pública, ca-certificates para HTTPS, net-tools para debugging
RUN apt-get update && \
    apt-get install -y \
        curl \
        ca-certificates \
        net-tools \
    && rm -rf /var/lib/apt/lists/*

# Copiar el ejecutable del servidor
COPY ./serverFileManager /serverFileManager

# Crear la carpeta que verá el cliente en el servidor
RUN mkdir -p /FileManagerDir && chmod -R 777 /FileManagerDir

# Dar permisos de ejecución al binario
RUN chmod +x /serverFileManager

# Exponer el puerto del servidor
EXPOSE 32001

# El comando y args se sobrescriben en el deployment de Kubernetes
# pero dejamos un CMD por defecto por si se ejecuta manualmente
CMD ["/serverFileManager"]


