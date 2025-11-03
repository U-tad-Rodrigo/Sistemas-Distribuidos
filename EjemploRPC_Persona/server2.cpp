
#include "utils.h"
#include <string>

using namespace std;

void atiendeCliente(int clientID) {

    int datoRecicibido = -1;

    // Recibir datos
        recvMSG(clientID, buffer);
        // desempaquetar datoRecibido
        datoRecicibido = unpack<int>(buffer);

        string mensajeRecibido;
        // Sacar tamano del mensaje y redimensionarlo
        mensajeRecibido.resize(unpack<int>(buffer));
        // Extraer datos
        unpackv(buffer, (char*)mensajeRecibido.data(), (int) mensajeRecibido.size());



        cout<< "Dato recibido del cliente: " << datoRecicibido << endl;

        bool correcto = true;

    // Enviar resultado
        // Limpiar el buffer
        buffer.clear();
        // Empaquetamos el booleano
        pack(buffer, correcto);
        sendMSG(clientID, buffer);


    // Cerrar conexion cliente
    closeConnection(clientID);

}

int main(int argc, char* argv[]) {

    vector<unsigned char> buffer;


    // Recibir datos de cliente
        // Iniciar server
        int serverConn = initServer(5553);

    // Mientras no cerrar
    while (1) {
        // Esperar conexion --> Es una barrera activa
        while (!checkClient()) { // 1 --> Si hay cliente esperando | 0 --> No hay clientes
            usleep(100);
        }
        int clientID = getLastClientID(); // Recuperar identificador de conexion del cliente.
        cout << "Cliente conectado" << endl;

        // atender cliente
        thread* th = new thread(atiendeCliente, clientID);
    }





        // Apagar server
        close(serverConn);


    return 0;
}
