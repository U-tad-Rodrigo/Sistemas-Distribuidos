
#include "utils.h"
#include <string>

using namespace std;

int main(int argc, char* argv[]) {
    vector<unsigned char> buffer;
    int dato = 738; // Quiero enviar ese dato al server
    string mensaje = "Hola mundo";

    // Enviar datos a servidor:

    // Crear conexion
        int serverID = initClient("127.0.0.1", 5553).serverId;
        // Crear paquete de datos
            // Empaquetar datos
            pack(buffer, dato);

            // Empaquetar datos complejos
                // Pack metadatos (descripcion)
                pack(buffer, (int)mensaje.size());
                // Pack de datos
                    // string --> array
                packv(buffer, (char *)mensaje.data(), (int) mensaje.size());
        // Enviar
            sendMSG(serverID, buffer);
        // Recibir resultado
            // Limpiar Buffer
            buffer.clear();
            recvMSG(serverID, buffer);

        cout << "Recibido: " << unpack<bool>(buffer) << endl;
        // Cerrar conexion
        closeConnection(serverID);

    return 0;
}
