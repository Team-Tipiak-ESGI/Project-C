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

    writePacket(ssl, USERNAME_RECEIVED, NULL);

    if (verifyUser(client->username, client->password, mongoConnection)) {
        writePacket(ssl, LOGGED_IN, NULL);
    } else {
        writePacket(ssl, UNAUTHORIZED, NULL);
    }
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

    writePacket(ssl, PASSWORD_RECEIVED, NULL);

    if (verifyUser(client->username, client->password, mongoConnection)) {
        writePacket(ssl, LOGGED_IN, NULL);
    } else {
        writePacket(ssl, UNAUTHORIZED, NULL);
    }
}

void CreateUser(Client *client, ServerConfiguration *server, SSL * ssl, MongoConnection *mongoConnection, const char * content) {
    if (client->username != NULL && client->password != NULL) {
        int res = MongoConnection__createUser(mongoConnection, client->username, client->password);

        if (res == 1) {
            writePacket(ssl, USER_CREATED, NULL);
        } else {
            writePacket(ssl, USER_EXISTS, NULL);
        }
    } else {
        writePacket(ssl, UNKNOWN_ERROR, NULL);
    }
}

// TODO: Verify if file does not exists
void CreateFile(Client *client, ServerConfiguration *server, SSL * ssl, MongoConnection *mongoConnection, const char * content) {
    if (verifyUser(client->username, client->password, mongoConnection)) {
        client->filePath = createFile(client->username, client->password, content, server->rootDir);

        // If not already in database
        if (MongoConnection__getFilePath(mongoConnection, client->username, client->password, content) == NULL) {

            // Save real name (sent by the client) and hashed name (the one on the server) in the database
            MongoConnection__addFile(mongoConnection, client->username, client->password, content, client->filePath);

        }

        printf("File opened.\n");

        writePacket(ssl, FILE_CREATED, NULL);
    } else {
        writePacket(ssl, UNAUTHORIZED, NULL);
    }
}

void FileSize(Client *client, ServerConfiguration *server, SSL * ssl, MongoConnection *mongoConnection, const char * content) {
    if (verifyUser(client->username, client->password, mongoConnection)) {
        client->fileSize = strtol(content, NULL, 10);
        printf("File size is %d\n", client->fileSize);
    } else {
        writePacket(ssl, UNAUTHORIZED, NULL);
    }
}

void FileContent(Client *client, ServerConfiguration *server, SSL * ssl, MongoConnection *mongoConnection, const char * content) {
    if (verifyUser(client->username, client->password, mongoConnection)) {
        if (client->filePath != NULL) {
            // Append to file
            writeChunk(client->filePath, content, client->chunkSent);

            if (++client->chunkSent >= client->fileSize / CHUNK_SIZE) {
                printf("File sending is done\n");
                client->filePath = NULL;
            }

            writePacket(ssl, CHUNK_RECEIVED, NULL);

            printf("User %s sent file data\n", client->username);
        } else {
            printf("No file is currently open\n");
            writePacket(ssl, UNKNOWN_ERROR, NULL);
        }
    } else {
        writePacket(ssl, UNAUTHORIZED, NULL);
        printf("User is not logged in!\n");
    }
}

void ReadFile(Client *client, ServerConfiguration *server, SSL * ssl, MongoConnection *mongoConnection, const char * content) {
    if (verifyUser(client->username, client->password, mongoConnection)) {
        printf("User %s requested to read file %s\n", client->username, content);

        const char *path = MongoConnection__getFilePath(mongoConnection, client->username, client->password, content);

        struct dirent *dir, *dir2;
        int fileCount = 0;

        DIR *d = opendir(path);
        if (d) {
            while ((dir = readdir(d)) != NULL)
                if (dir->d_type == DT_REG) /* If the entry is a regular file */
                    fileCount++;

            // Send file size in chunk count
            char * packetContent = malloc(10);
            sprintf(packetContent, "%c", fileCount);
            writePacket(ssl, FILE_SIZE, packetContent);

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

                    writePacket(ssl, FILE_CONTENT, fileBuffer);
                }
            }

            closedir(d);
        }
    } else {
        writePacket(ssl, UNAUTHORIZED, NULL);
    }
}

void DeleteFile(Client *client, ServerConfiguration *server, SSL * ssl, MongoConnection *mongoConnection, const char * content) {
    if (verifyUser(client->username, client->password, mongoConnection)) {
        printf("User %s requested to delete file %s\n", client->username, content);

        const char *path = MongoConnection__getFilePath(mongoConnection, client->username, client->password, content);

        if (path == NULL) {
            writePacket(ssl, DELETE_ERROR, NULL);
            return;
        }

        printf("Deleting file %s\n", path);

        struct dirent *dir;
        DIR *d = opendir(path);
        if (d) {
            while ((dir = readdir(d)) != NULL) {
                if (dir->d_type == DT_REG) {
                    char *fullPath = malloc(sizeof path + sizeof dir->d_name + 1);
                    strcpy(fullPath, path);
                    strcat(fullPath, "/");
                    strcat(fullPath, dir->d_name);
                    remove(fullPath);
                    free(fullPath);
                }
            }

            rmdir(path);

            MongoConnection__deleteFile(mongoConnection, client->username, client->password, path);

            writePacket(ssl, FILE_DELETED, NULL);
        } else {
            writePacket(ssl, DELETE_ERROR, NULL);
        }
    } else {
        writePacket(ssl, UNAUTHORIZED, NULL);
    }
}

void ListFiles(Client *client, ServerConfiguration *server, SSL * ssl, MongoConnection *mongoConnection, const char * content) {
    if (verifyUser(client->username, client->password, mongoConnection)) {
        printf("User %s requested file listing\n", client->username);

        char * files = MongoConnection__listFile(mongoConnection, client->username, client->password);
        writePacket(ssl, LIST_FILES, files);
    } else {
        writePacket(ssl, UNAUTHORIZED, NULL);
    }
}
