#include "client.h"
#include "utils.h"
#include <vector>
using namespace std;

int main(int argc, char** argv) {
    vector<unsigned char> buffer;
    int dato=738;
    string mensaje = "Hola";

    int serverId= initClient("127.0.0.1", 5553).serverId;
    pack(buffer, dato);
    pack(buffer, (int)mensaje.size());
    packv(buffer, (char*)mensaje.data(), mensaje.size());

    buffer.clear();
    recvMSG(serverId, buffer);

    cout << "Dato recibido: " << unpack<int>(buffer) << endl;
    closeConnection(serverId);

    return 0;

}