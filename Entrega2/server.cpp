#include "utils.h"
#include <iostream>
#include <string>
#include <thread>
#include <list>

using namespace std;

void atiendeConexion(int clientId)
{
    vector<unsigned char> buffer;
    string mensaje;

    cout << "Nuevo cliente conectado (ID: " << clientId << ")" << endl;

    // Verificar que el cliente existe en clientList
    if(clientList.find(clientId) == clientList.end())
    {
        cout << "ERROR: Cliente " << clientId << " no encontrado en clientList" << endl;
        return;
    }

    cout << "Socket del cliente: " << clientList[clientId].socket << endl;
    cout << "Esperando datos del cliente..." << endl;

    try {
        recvMSG(clientId, buffer);

        cout << "Datos recibidos, tamano del buffer: " << buffer.size() << " bytes" << endl;

        if(buffer.size() == 0)
        {
            cout << "Cliente desconectado antes de enviar datos" << endl;
            closeConnection(clientId);
            return;
        }

        mensaje.resize(unpack<int>(buffer));

        unpackv<char>(buffer, (char*)mensaje.data(), mensaje.size());

        cout << "Mensaje recibido: \"" << mensaje << "\"" << endl;

        closeConnection(clientId);

        cout << "Conexion cerrada correctamente" << endl;
    }
    catch(...) {
        cout << "Error al procesar cliente" << endl;
        closeConnection(clientId);
    }
}

int main(int argc, char** argv)
{
    cout << "Iniciando servidor en puerto 3000..." << endl;
    auto conn = initServer(3000);
    cout << "Servidor listo y esperando conexiones..." << endl;
    cout << "Presiona Ctrl+C para detener el servidor" << endl;
    cout << "----------------------------------------" << endl;

    while(1)
    {
        while(!checkClient()) usleep(100);

        int clientId = getLastClientID();

        thread* th = new thread(atiendeConexion, clientId);
        th->detach();
    }

    close(conn);

    return 0;
}