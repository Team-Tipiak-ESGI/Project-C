#include "Item.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Item * Configuration__loadFromFile(char * filepath) {
    // Read file
    FILE* file;
    char* buffer;
    long fileLength;

    // open file in binary mode
    file = fopen(filepath, "rb");

    if (file == NULL) {
        printf("Config file not found, creating it...\n");
        file = fopen(filepath, "wb");

        fputs("port:8100\naddress:localhost\nrootDir:server\ncertFile:tucs.cert\nkeyFile:tucs.key\ndbURI:mongodb://localhost:27017\0", file);

        fclose(file);

        file = fopen(filepath, "rb");
    }

    // jump to end of file
    fseek(file, 0, SEEK_END);
    // get offset (used for length)
    fileLength = ftell(file);
    // go back to start of file
    rewind(file);

    // allocate memory for the buffer
    buffer = (char*)malloc(fileLength * sizeof(char));
    // write file data to buffer
    fread(buffer, 1, fileLength, file);
    // close file
    fclose(file);

    // Load config
    char * tok = buffer;

    Item *item = malloc(sizeof(Item));
    item->next = NULL;

    // Print the file list
    while ((tok = strtok(tok, "\r\n")) != NULL) {
        char * value = strdup(tok);
        char * key = strsep(&value, ":");
        //printf("Key:[%s]\n  Value:[%s]\n", key, value);

        Item__insert(item, key, value);

        tok = NULL;
    }

    //return item;
    return item;
}
