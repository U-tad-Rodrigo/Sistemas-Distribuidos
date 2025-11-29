//
// Created by Rodrigo on 14/11/2025.
//

//Prueba de broker
#include <iostream>
#include "brokerManager.h"
#include <thread>

using namespace std;

#define BROKER_PORT 1066

int main(int argc, char** argv) {
    bool exit = false;
    int brokerPortId = initServer(BROKER_PORT);
    while (!exit) {
        while (!checkClient()) usleep(100);
        int idCliente = getLastClientID();

        auto* th = new thread(brokerManager::resolveBrokerMessages, idCliente);
        th->detach();
    }
}