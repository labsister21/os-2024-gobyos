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
void cp(char* fileName);
void rm(char* fileName);
void ls();
void mkdir(char* fileName);
void cd(char* fileName);
void mv(char* fileName);
void find(char* fileName);
void kill(char* fileName);
void exec(char* fileName);
void ps();
void processDFS (char srcName[8], uint32_t search_directory_number, int v, bool visited[63]);
void clock();

#endif