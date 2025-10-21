#include "utils.h"
#include <string>
#include <iostream>
#include <unistd.h>

using namespace std;

int main(int argc,char** argv)
{
    vector<unsigned char> buffer;
    string mensaje = "Hola mundo";

    cout << "Conectando al servidor..." << endl;

    auto conn = initClient("127.0.0.1", 3000);

    cout << "Resultado de conexion - Socket: " << conn.socket << ", Alive: " << conn.alive << endl;

    if(conn.socket == -1 || !conn.alive)
    {
        cout << "Error: No se pudo conectar al servidor" << endl;
        cout << "Asegurate de que el servidor este ejecutandose en el puerto 3000" << endl;
        return 1;
    }

    cout << "Conexion establecida exitosamente" << endl;
    cout << "Socket: " << conn.socket << ", Server ID: " << conn.serverId << endl;

    // Empaquetar el tamaño del mensaje
    pack(buffer, (int)mensaje.size());

    // Empaquetar el contenido del mensaje
    packv(buffer, (char*)mensaje.data(), mensaje.size());

    cout << "Buffer preparado, tamano: " << buffer.size() << " bytes" << endl;

    // Enviar el mensaje al servidor
    cout << "Enviando mensaje..." << endl;
    sendMSG(conn.serverId, buffer);

    cout << "Mensaje enviado al servidor: " << mensaje << endl;

    // Pequeño delay antes de cerrar para asegurar que el mensaje se envíe
    sleep(5);

    // Cerrar la conexión
    cout << "Cerrando conexion..." << endl;
    closeConnection(conn.serverId);

    cout << "Conexion cerrada" << endl;

    return 0;
}