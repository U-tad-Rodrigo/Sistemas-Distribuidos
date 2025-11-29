# Autor: Rodrigo - Práctica 2 Sistemas Distribuidos
# Dockerfile para brokerFileManager

FROM ubuntu:20.04

WORKDIR /app

# Copiar el ejecutable del broker al contenedor
COPY brokerFileManager /app/brokerFileManager

# Dar permisos de ejecución
RUN chmod +x /app/brokerFileManager

# Exponer el puerto del broker
EXPOSE 32002

# Comando por defecto para ejecutar el broker
CMD ["./brokerFileManager"]

