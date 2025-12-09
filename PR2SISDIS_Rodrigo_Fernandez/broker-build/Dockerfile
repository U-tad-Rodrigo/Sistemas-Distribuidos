# Autor: Rodrigo - Práctica 2 Sistemas Distribuidos
# Dockerfile para brokerFileManager

FROM ubuntu:22.04

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        ca-certificates \
        net-tools \
    && rm -rf /var/lib/apt/lists/*

COPY brokerFileManager /brokerFileManager
RUN chmod +x /brokerFileManager

EXPOSE 32002

CMD ["/brokerFileManager"]

