#include <stdio.h>

typedef struct {
    FILE* file;
    size_t size;
    char* buffer;
} FileManager;

void destroyFileManager(FileManager* fileManager);
void readFile(FileManager* fileManager);
FileManager createFileManager(char* fileName);