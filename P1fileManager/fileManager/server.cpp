/*Rodrigo Fernández
* 05/11/2205
 */

#include "clientManager.h"
#include "utils.h"
#include <iostream>
#include <string>
#include <thread>
#include <list>

using namespace std;

int main(int argc, char** argv)
{
 bool exit = false;
 cout<<"Server opening port\n";
 int serverPortId= initServer(1067);
 cout<<"Server port opened, waiting for connections\n";
 while(!exit) {
  while (!checkClient()) usleep(100);
  int clientId = getLastClientID();
  cout<<"Client ID: "<<clientId<<" connected\n";

  thread* th = new thread(clientManager::resolveClientMessages, clientId);

 }

 return 0;
}
