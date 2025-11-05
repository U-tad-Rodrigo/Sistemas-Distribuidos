/*Rodrigo Fernández
 * 05/11/2025
 */

#include "utils.h"
#include <iostream>
#include <string>
#include <map>
#include <vector>

#define ERRLOG(msg) cout<<"Error "<<__FILE__<<":"<<__LINE__<<" "<<msg<<" \n"

using namespace std;

class clientManager{

public:

	typedef enum{
		texto,
		usuarios,
		privado,
		exit,
		ack
	}msgTypes;

	static mutex cerrojoClientes;
	static map<int,int> clientesConectados;

	static void atiendeCliente(int clientId);
	static string desempaquetaTexto(vector<unsigned char> &buffer);
	static void reenviaTexto(int idEmisor, string msg);
	static void enviaListaUsuarios(int clientId);
	static void enviaMensajePrivado(int idEmisor, int idDestinatario, string msg);
	static string obtenerListaIds();

};

