/*Rodrigo Fernández
* 07/11/2025
 */
#pragma once

#include "utils.h"
#include <string>
#include <map>

using namespace std;

typedef enum{
    SERVER_CONNECT,
    CLIENT_CONNECT,
    ACK_BROKER
}brokerMsgTypes;

typedef struct {
    string ip;
    int port;
    int numClients;
    int keepAliveCounter;
    bool alive;
}ServerInfo;

class brokerManager {
public:
    static inline map<int, ServerInfo> servidoresRegistrados;

    static void resolveBrokerMessages(int connectionId);
    static int findServerWithLessClients();
    static void keepAliveMonitor();
};

