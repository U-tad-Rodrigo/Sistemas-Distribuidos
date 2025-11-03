#ifndef EJEMPLORPC_PERSONA_PERSONADISTRIBUIDA_H
#define EJEMPLORPC_PERSONA_PERSONADISTRIBUIDA_H

#include "persona.h"
#include <vector>

void pack(Persona &p, std::vector<unsigned char> &buffer);
void unpack(Persona &p, std::vector<unsigned char> &buffer);

#endif //EJEMPLORPC_PERSONA_PERSONADISTRIBUIDA_H

