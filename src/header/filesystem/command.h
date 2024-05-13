#ifndef COMMANDS_H
#define COMMANDS_H

#include "../stdlib/stdtype.h"
#include "../stdlib/string.h"
#include "../stdlib/stdmem.h"
#include "fat32.h"


struct CWDdata {
    uint32_t currentCluster;
    char cwdName[8];
    uint32_t prevCluster;
};

void splitname(char* buf, char* first, char* second, int offset);
void cat(char* fileName);

#endif