#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>

#include <unistd.h> /* close */
#include <netdb.h> /* gethostbyname */

#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/sha.h>
#include <openssl/md5.h>

#include "../shared/PacketTypes.h"
#include "../shared/ChunkSize.h"
#include "ServerConfiguration.h"
#include "Client.h"
#include "MongoConnection.h"
#include "Database.h"

/**
 * Verify if the given credentials are valid
 * @param username
 * @param password
 * @return 1 if valid, 0 if not
 */
unsigned char verifyUser(char* username, char* password, MongoConnection* mongoConnection) {
    if (username == NULL && password == NULL) {
        return 0;
    }

    // TODO: Verify using values in database
    const char* validUsername = "quozul\0";
    const char* validPassword = "password\0";

    return (strcmp(username, validUsername) == 0 && strcmp(password, validPassword) == 0);
}

// Save file's chunks in a folder
//   (folder) a7e9fb
//     - (file) 1 (← this is the chunk number)
//     - (file) 2
//     - (file) 3
// TODO: Save file's information in the database
//   - owner
//   - chunk count
//   - chunk size

/**
 * Create a folder to store all file's chunks
 * @param username Username of the owner
 * @param password Password of the owner (should be hashed)
 * @param fileName Send file name by the client
 * @param rootDir Directory to store files from server's configuration
 * @return Path of the folder which should contain all the chunks of the file
 */
char* createFile(const char* username, const char* password, const char* fileName, const char* rootDir) {
    char hashedName[40] = {0};
    char * filePath;
    char * temp = malloc(1024);

    char * hashInput = malloc(1024);
    unsigned char hashedOutput[20];

    // Generate hashed name for the file, should be unique
    strcpy(temp, username);
    strtok(temp, "\0");
    strcpy(hashInput, temp);

    strcpy(temp, password);
    strtok(temp, "\0");
    strcat(hashInput, temp);

    free(temp);

    strcat(hashInput, fileName);

    SHA1((const unsigned char*)hashInput, strlen(hashInput), hashedOutput);

    // Convert SHA to hex string
    char hex[3];
    for (int i = 0; i < 20; i++) {
        sprintf(hex, "%02x", hashedOutput[i]);
        strcat(hashedName, hex);
    }

    // Generate file path
    filePath = malloc(strlen(rootDir) + strlen(hashedName) + 1);
    strcpy(filePath, rootDir);
    strcat(filePath, hashedName);

    // Create directory if not exists
    struct stat st = {0};

    if (stat(filePath, &st) == -1) {
        mkdir(filePath, 0700);
    }

    return filePath;
}

void writeChunk(const char* originalFilePath, const char* content, const int chunkNumber) {
    char chunkId[12];
    char filePath[1024];
    sprintf(chunkId, "%d", chunkNumber);

    strcpy(filePath, originalFilePath);
    strcat(filePath, "/");
    strcat(filePath, chunkId);

    printf("%s\n", filePath);

    FILE* file = fopen(filePath, "wb+");

    fputs(content, file);
    printf("Data written\n");

    fclose(file);
}

/**
 * Serve the connection
 * @param ssl
 */
void servlet(SSL *ssl, ServerConfiguration server, MongoConnection* mongoConnection) {
    char buffer[CHUNK_SIZE];
    char writeBuffer[CHUNK_SIZE];
    int sd, bytes;

    if (SSL_accept(ssl) < 0) {     /* do SSL-protocol accept */
        ERR_print_errors_fp(stderr);
    } else {
        Client client;
        client.chunkSent = 0;

        while (1) {
            bytes = SSL_read(ssl, buffer, CHUNK_SIZE); /* get request */

            if (bytes == 0) break;

            const unsigned char firstByte = buffer[0];
            const char* content = buffer + 1;

            printf("Message from socket (%d) (packet type: %d) : %s\n", bytes, firstByte, content);

            switch (firstByte) {
                // TODO: Add option to create new user
                case USERNAME:
                    // Copy username to variable
                    client.username = malloc(sizeof(char) * strlen(content));
                    strncpy(client.username, content, strlen(content));
                    break;

                case PASSWORD:
                    // Copy password to variable
                    client.password = malloc(sizeof(char) * strlen(content));
                    strncpy(client.password, content, strlen(content));
                    break;

                case CREATE_FILE:
                    client.filePath = createFile(client.username, client.password, content, server.rootDir);
                    printf("File opened\n");
                    break;

                case FILE_SIZE:
                    client.fileSize = strtol(content, NULL, 10);
                    printf("File size is %d\n", client.fileSize);
                    break;

                case FILE_CONTENT:
                    if (verifyUser(client.username, client.password, mongoConnection)) {
                        if (client.filePath != NULL) {
                            // Append to file
                            writeChunk(client.filePath, content, client.chunkSent);

                            if (++client.chunkSent >= client.fileSize / CHUNK_SIZE) {
                                printf("File sending is done\n");
                                client.filePath = NULL;
                            }

                            sprintf(writeBuffer, "%c", CHUNK_RECEIVED);
                            SSL_write(ssl, writeBuffer, CHUNK_SIZE);

                            printf("User %s sent file data\n", client.username);
                        } else {
                            printf("No file is currently open\n");
                        }
                    } else {
                        printf("User is not logged in!\n");
                    }
                    break;

                default:
                    // HTTPS response
                    SSL_write(ssl, "HTTP/1.1 405 Method Not Allowed\n"
                                   "Content-Length: 33\n"
                                   "Content-Type: text/html;\n"
                                   "\n"
                                   "405 This is not a http(s) server!", CHUNK_SIZE);
                    break;
            }
        }
    }

    sd = SSL_get_fd(ssl);       /* get socket connection */
    SSL_free(ssl);         /* release SSL state */
    close(sd);          /* close connection */

    printf("Connection closed.\n");
}
