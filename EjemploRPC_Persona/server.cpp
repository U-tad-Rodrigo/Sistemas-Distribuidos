#include "utils.h"
#include <iostream>
#include <thread>
#include "clientManager.h"

// crear conexion:

    // initClient(IP, PORT) // Conecta un cliente con un servidor

    // initServer(PORT) // Crea server que escucha en ese puerto

    // sendMSG(id, buffer)
        // id = identificador destino (cliente o servidor)
        // buffer = Datos a enviar

    // recvMSG(id, buffer)
        // id = identificador origen (cliente o servidor)
        // buffer = Datos a recibir


int main(int argc, char** argv)
{
	
	//init server
	int serverSocketID=initServer(5553);

	//esperar conexion
	while(1){
		while(!checkClient()) usleep(100);
		
		std::cout<<"Cliente conectado\n";
		
		int clientId=getLastClientID();
		std::thread *th=new std::thread( clientManager::atiendeCliente,clientId);
	}
	
	//cerrar
	close(serverSocketID);
    return 0; 
}
