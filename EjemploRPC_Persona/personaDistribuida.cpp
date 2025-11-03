#include "persona.h"
#include "clientManager.h"

#define IP_SERVER "127.0.0.1"
#define PORT_SERVER 5000

Persona::Persona(){

    // Conectar Server
    int serverID = initClient(IP_SERVER, PORT_SERVER).serverId;

    // Crear mensaje
    vector<unsigned char> buffer;

        // tipo de mensaje --> ConstruyePersona
        pack(buffer, construyePersona);
        // Parametros de entrada --> None

        // Parametros de salida --> ACK/Exito

    // Enviar paquete
    sendMSG(serverID, buffer);

    // Recibir parametros salida (resultado)
    buffer.clear();
    recvMSG(serverID, buffer);

    if (unpack<personaFuncs>(buffer) != ackMSG) { // DESEMPAQUETA ACK Y SI ES DISTINTO DEL ACK
        cout << "ERROR: Respuesta no valida " << __FILE__ << ":" << __LINE__ << endl;
    }

    // Finalizar
        // Guardar datos de conexion
        clientManager::connectionIds[this] = serverID;

}

Persona::Persona(string nombre, int edad){

    // Conectar Server
    int serverID = initClient(IP_SERVER, PORT_SERVER).serverId;

    // Crear mensaje
    vector<unsigned char> buffer;

    // tipo de mensaje --> ConstruyePersonaParams
    pack(buffer, construyePersonaParams);
    // Parametros de entrada --> nombre(string), edad(int)
        // Empaqueto la longitud del nombre
        pack(buffer, (int)nombre.size());
        // Empaqueto el contenido del nombre
        packv(buffer, (char*) nombre.data(), (int)nombre.size());
        // Empaqueto la edad
        pack(buffer, edad);

    // Enviar paquete
    sendMSG(serverID, buffer);

    // Recibir parametros salida (resultado)
    buffer.clear();
    recvMSG(serverID, buffer);

    if (unpack<personaFuncs>(buffer) != ackMSG) { // DESEMPAQUETA ACK Y SI ES DISTINTO DEL ACK
        cout << "ERROR: Respuesta no valida " << __FILE__ << ":" << __LINE__ << endl;
    }

    // Finalizar
    // Guardar datos de conexion
    clientManager::connectionIds[this] = serverID;
}
Persona::~Persona(){

    // Crear mensaje
        // tipo de mensaje --> DestroyPersona
        // Parametros de entrada --> None
        // Parametros de salida --> ACK/Exito

    // Enviar paquete

    // Recibir parametros salida (resultado)

    // Finalizar

    // Desconectar Server

}
	
void Persona::setNombre(string nombre){
}
void Persona::setEdad(int edad){
		
}
string Persona::getNombre(){
	
	return "";
		
	
}
int Persona::getEdad(){
	
	return 0;
	
	
}








