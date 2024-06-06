#include "io.h"

void destroyFileManager(FileManager* fileManager) {
    fclose(fileManager->file);
}

void readFile(FileManager* fileManager) {
    size_t bytesRead = fread(fileManager->buffer, 1, fileManager->size, fileManager->file);

    fileManager->buffer[fileManager->size] = '\0';

    if (bytesRead != fileManager->size) {
        printf("%zu\n", bytesRead);
        fprintf(stderr, "Error opening file\n");
    }
}

FileManager createFileManager(char* fileName) {
    FileManager fileManager = {};
    FILE* file = fopen(fileName, "r");

    if (!file) {
        printf("Error opening the file.\n");
    }

    fseek(file, 0, SEEK_END);

    fileManager.file = file;
    fileManager.size = ftell(file);

    fseek(file, 0, SEEK_SET);

    return fileManager;
}