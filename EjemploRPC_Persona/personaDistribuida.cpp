#include "persona.h"
#include "clientManager.h"

#define IP_SERVER "127.0.0.1"
#define PORT_SERVER 5553

Persona::Persona(){
	//conectar al server
	int serverId=initClient(IP_SERVER, PORT_SERVER).serverId;
	//crear mensaje
		//tipo de mensaje: ConstruyePersona
		vector< unsigned char> buffer;
		//empaquetar tipo de mensaje
			pack(buffer, construyePersona);
		//parametros de entrada: none
		//parametros de salida: ACK/exito
	//enviar mensaje
		sendMSG(serverId, buffer);
	//recibir parametros de salida(resultado)
	buffer.clear();
	recvMSG(serverId, buffer);

	//si no recibo ack error
	if (unpack<personaFuncs>(buffer) != ackMSG) //desempaqueta ack
		cout<<"Error Respuesta no valida"<<__FILE__<<":"<<__LINE__<<endl;

	//acabar
		//guardar datos de la conexion
		clientManager::connectionIds[this]= serverId;

}

Persona::Persona(string nombre, int edad){
	//conectar al server
	int serverId=initClient(IP_SERVER, PORT_SERVER).serverId;
	//crear mensaje
		//tipo de mensaje: ConstruyePersona
		vector< unsigned char> buffer;

		pack(buffer, construyePersonaParams);
		//parametros de entrada: nombre(string), edad(int)
		pack(buffer,(int)nombre.size());
		packv(buffer,(char*)nombre.data(),(int)nombre.size());

		pack(buffer,(int)edad);
		//parametros de salida: ACK/exito
		//empaquetar tipo de mensaje

	//enviar mensaje
		sendMSG(serverId, buffer);
	//recibir parametros de salida(resultado)
	buffer.clear();
	recvMSG(serverId, buffer);

	//si no recibo ack error
	if (unpack<personaFuncs>(buffer) != ackMSG) //desempaqueta ack
		cout<<"Error Respuesta no valida"<<__FILE__<<":"<<__LINE__<<endl;

	//acabar
		//guardar datos de la conexion
		clientManager::connectionIds[this]= serverId;
}
Persona::~Persona(){
	int serverId=clientManager::connectionIds[this];
	//crear mensaje
		//tipo de mensaje: DestruyePersona
		vector< unsigned char> buffer;
		pack(buffer, destruyePersona);
		//parametros de entrada: none
		//parametros de salida: ACK/exito
	//enviar paquete
	sendMSG(serverId, buffer);
	//recibir parametros de salida(resultado)
	buffer.clear();
	recvMSG(serverId, buffer);
	if (unpack<personaFuncs>(buffer) != ackMSG) //desempaqueta ack
		cout<<"Error Respuesta no valida"<<__FILE__<<":"<<__LINE__<<endl;

	//acabar
	//desconectar/cerrar conexion
	closeConnection(serverId);
}

void Persona::setNombre(string nombre){
	int serverId=clientManager::connectionIds[this];
	vector< unsigned char> buffer;

	//crear paquete
		//pack setNombreF
		pack(buffer,setNombreF);
		pack(buffer,(int)nombre.size());
		packv(buffer,(char*)nombre.data(),(int)nombre.size());
	//enviar paquete
		sendMSG(serverId, buffer);

	//acabar
	buffer.clear();
	recvMSG(serverId, buffer);

	//si no recibo ack error
	if (unpack<personaFuncs>(buffer) != ackMSG) //desempaqueta ack
		cout<<"Error Respuesta no valida"<<__FILE__<<":"<<__LINE__<<endl;

}
void Persona::setEdad(int edad){
	int serverId=clientManager::connectionIds[this];
	vector< unsigned char> buffer;

	//crear paquete
	//pack setEdadF
	pack(buffer,setEdadF);
	pack(buffer,(int)edad);
	//enviar paquete
	sendMSG(serverId, buffer);

	//acabar
	buffer.clear();
	recvMSG(serverId, buffer);

	//si no recibo ack error
	if (unpack<personaFuncs>(buffer) != ackMSG) //desempaqueta ack
		cout<<"Error Respuesta no valida"<<__FILE__<<":"<<__LINE__<<endl;

}
string Persona::getNombre(){
	int serverId=clientManager::connectionIds[this];
	vector< unsigned char> buffer;

	pack(buffer,getNombreF);
	sendMSG(serverId, buffer);
	buffer.clear();
	recvMSG(serverId, buffer);

	//recibir resultado
	string resultado;
	resultado.resize(unpack<int>(buffer));
	unpackv(buffer,(char*)resultado.data(),(int)resultado.size());

	if (unpack<personaFuncs>(buffer) != ackMSG)
		cout<<"Error Respuesta no valida"<<__FILE__<<":"<<__LINE__<<endl;

	return resultado;


}
int Persona::getEdad(){
	int serverId=clientManager::connectionIds[this];
	vector< unsigned char> buffer;

	pack(buffer,getNombreF);
	sendMSG(serverId, buffer);
	buffer.clear();
	recvMSG(serverId, buffer);

	//recibir resultado
	int resultado;
	resultado = unpack<int>(buffer);

	if (unpack<personaFuncs>(buffer) != ackMSG)
		cout<<"Error Respuesta no valida"<<__FILE__<<":"<<__LINE__<<endl;

	return resultado;


}