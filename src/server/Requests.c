#include <openssl/ssl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "MongoConnection.h"

#include "../shared/PacketTypes.h"
#include "../shared/ChunkSize.h"
#include "ServerConfiguration.h"
#include "Client.h"
#include "Utils.h"

#include "Database.h"

void Username(Client *client, ServerConfiguration *server, SSL * ssl, MongoConnection *mongoConnection, const char * content) {
    // Copy username to variable
    client->username = strdup(content);
}

void Password(Client *client, ServerConfiguration *server, SSL * ssl, MongoConnection *mongoConnection, const char * content) {
    // Hash password
    client->password = malloc(64);
    unsigned char * hashedPassword = malloc(32);

    // Set password to all 0
    for (int i = 0; i < 64; i++) client->password[i] = 0;
    for (int i = 0; i < 32; i++) hashedPassword[i] = 0;

    // Generate hash
    SHA256((const unsigned char*)content, strlen(content), hashedPassword);

    // Convert hash to hex
    char hex[3];
    for (int i = 0; i < 32; i++) {
        sprintf(hex, "%02x", hashedPassword[i]);
        strcat(client->password, hex); // Copy hex to client's password
    }

    free(hashedPassword);
}

void CreateUser(Client *client, ServerConfiguration *server, SSL * ssl, MongoConnection *mongoConnection, const char * content) {
    char writeBuffer[CHUNK_SIZE];
    if (client->username != NULL && client->password != NULL) {
        int res = MongoConnection__createUser(mongoConnection, client->username, client->password);

        if (res == 1) {
            sprintf(writeBuffer, "%c", USER_CREATED);
        } else {
            sprintf(writeBuffer, "%c", USER_EXISTS);
        }

        SSL_write(ssl, writeBuffer, CHUNK_SIZE);
    }
}

void CreateFile(Client *client, ServerConfiguration *server, SSL * ssl, MongoConnection *mongoConnection, const char * content) {
    client->filePath = createFile(client->username, client->password, content, server->rootDir);
    // Save real name (sent by the client) and hashed name (the one on the server) in the database
    MongoConnection__addFile(mongoConnection, client->username, client->password, content, client->filePath);
    printf("File opened\n");
}

void FileSize(Client *client, ServerConfiguration *server, SSL * ssl, MongoConnection *mongoConnection, const char * content) {
    client->fileSize = strtol(content, NULL, 10);
    printf("File size is %d\n", client->fileSize);
}

void FileContent(Client *client, ServerConfiguration *server, SSL * ssl, MongoConnection *mongoConnection, const char * content) {
    char writeBuffer[CHUNK_SIZE];
    if (verifyUser(client->username, client->password, mongoConnection)) {
        if (client->filePath != NULL) {
            // Append to file
            writeChunk(client->filePath, content, client->chunkSent);

            if (++client->chunkSent >= client->fileSize / CHUNK_SIZE) {
                printf("File sending is done\n");
                client->filePath = NULL;
            }

            sprintf(writeBuffer, "%c", CHUNK_RECEIVED);
            SSL_write(ssl, writeBuffer, CHUNK_SIZE);

            printf("User %s sent file data\n", client->username);
        } else {
            printf("No file is currently open\n");
        }
    } else {
        sprintf(writeBuffer, "%c", UNAUTHORIZED);
        SSL_write(ssl, writeBuffer, CHUNK_SIZE);
        printf("User is not logged in!\n");
    }
}

void ReadFile(Client *client, ServerConfiguration *server, SSL * ssl, MongoConnection *mongoConnection, const char * content) {
    char writeBuffer[CHUNK_SIZE];
    printf("User %s requested to read file %s\n", client->username, content);

    char * userDir = getUserDir(client->username, client->password);
    char * path = malloc(sizeof server->rootDir + sizeof userDir + sizeof content + 2);

    strcpy(path, server->rootDir);
    strcat(path, userDir);
    strcat(path, "/");
    strcat(path, content); // TODO: Adapt using the original name

    struct dirent *dir, *dir2;
    int fileCount = 0;

    DIR * d = opendir(path);
    if (d) {
        while ((dir = readdir(d)) != NULL)
            if (dir->d_type == DT_REG) /* If the entry is a regular file */
                fileCount++;

        // Send file size in chunk count
        sprintf(writeBuffer, "%c%c", FILE_SIZE, fileCount);
        SSL_write(ssl, writeBuffer, CHUNK_SIZE);

        rewinddir(d);

        // TO FIX: STUCK HERE
        while ((dir2 = readdir(d)) != NULL) {
            if (dir2->d_type == DT_REG) {
                char *filePath = malloc(sizeof(path) + sizeof(dir->d_name));
                strcpy(filePath, path);
                strcat(filePath, "/");
                strcat(filePath, dir2->d_name);

                // Load file content into memory
                FILE *file = fopen(filePath, "rb");
                char *fileBuffer;
                long fileLength;

                // jump to end of file
                fseek(file, 0, SEEK_END);
                // get offset (used for length)
                fileLength = ftell(file);

                // go back to start of file
                rewind(file);

                // allocate memory for the fileBuffer
                fileBuffer = (char *) malloc(fileLength * sizeof(char));
                // write file data to fileBuffer
                fread(fileBuffer, 1, fileLength, file);
                // close file
                fclose(file);

                sprintf(writeBuffer, "%c%s", FILE_CONTENT, fileBuffer);
                SSL_write(ssl, writeBuffer, CHUNK_SIZE);
            }
        }

        closedir(d);
    }
}

// TODO: File delete
void DeleteFile(Client *client, ServerConfiguration *server, SSL * ssl, MongoConnection *mongoConnection, const char * content) {
    printf("User %s requested to delete file %s\n", client->username, content);

    char * userDir = getUserDir(client->username, client->password);
    char * path = malloc(sizeof server->rootDir + sizeof userDir + sizeof content + 2);

    strcpy(path, server->rootDir);
    strcat(path, userDir);
    strcat(path, "/");
    strcat(path, content); // TODO: Adapt using the original name

    printf("Deleting file %s\n", path);

    struct dirent *dir;
    DIR * d = opendir(path);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_type == DT_REG) {
                char * fullPath = malloc(sizeof path + sizeof dir->d_name + 1);
                strcpy(fullPath, path);
                strcat(fullPath, "/");
                strcat(fullPath, dir->d_name);
                remove(fullPath);
            }
        }

        rmdir(path);
    }
}

// TODO: Use database
void ListFiles(Client *client, ServerConfiguration *server, SSL * ssl, MongoConnection *mongoConnection, const char * content) {
    char writeBuffer[CHUNK_SIZE];
    printf("User %s requested file listing\n", client->username);

    if (verifyUser(client->username, client->password, mongoConnection)) {
        printf("User %s requested file listing\n", client->username);

        //MongoConnection__listFile(mongoConnection, client->username, client->password);

        sprintf(writeBuffer, "%c", LIST_FILES);

        // List files in directory
        DIR *d;
        struct dirent *dir;

        // Generate file path
        char * hashedNameUser = getUserDir(client->username, client->password);
        char * filePath = malloc(strlen(server->rootDir) + strlen(hashedNameUser) + 1);
        strcpy(filePath, server->rootDir);
        strcat(filePath, hashedNameUser);

        d = opendir(filePath);
        if (d) {
            while ((dir = readdir(d)) != NULL) {
                if (dir->d_name[0] == '.') continue;

                // TODO: Get real file name
                sprintf(writeBuffer, "%s%s%c", writeBuffer, dir->d_name, 1);
            }
            closedir(d);
        }

        free(filePath);

        printf("Listing done!\n");
        SSL_write(ssl, writeBuffer, CHUNK_SIZE);
    }
}
