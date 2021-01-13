#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h> /* close */
#include <netdb.h> /* gethostbyname */

#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/sha.h>
#include <openssl/md5.h>

#include "../shared/PacketTypes.h"
#include "../shared/hash.h"
#include "ServerConfiguration.h"
#include "Client.h"
#define CHUNK_SIZE 1024

/**
 * Verify if the given credentials are valid
 * @param username
 * @param password
 * @return 1 if valid, 0 if not
 */
unsigned char verifyUser(char* username, char* password) {
    if (username == NULL && password == NULL) {
        return 0;
    }

    // TODO: Verify using values in database
    const char* validUsername = "quozul\0";
    const char* validPassword = "password\0";

    return (strcmp(username, validUsername) == 0 && strcmp(password, validPassword) == 0);
}

FILE* createFile(const char* username, const char* password, const char* fileName, const char* rootDir) {
    char hashedName[40] = {0};
    char * filePath;
    char * temp = malloc(1024);

    char * hashInput = malloc(1024);
    unsigned char hashedOutput[20];
    FILE * file;

    // Generate hashed name for the file, should be unique
    strcpy(temp, username);
    strtok(temp, "\0");
    strcpy(hashInput, temp);

    strcpy(temp, password);
    strtok(temp, "\0");
    strcat(hashInput, temp);

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

    // Delete existing file
    remove(filePath);

    // Open file
    file = fopen(filePath, "ab+");

    free(filePath);
    free(temp);

    return file;
}

/**
 * Serve the connection
 * @param ssl
 */
void servlet(SSL *ssl, ServerConfiguration server) {
    char buffer[CHUNK_SIZE];
    int sd, bytes;

    if (SSL_accept(ssl) < 0) {     /* do SSL-protocol accept */
        ERR_print_errors_fp(stderr);
    } else {
        Client client;
        client.chunkSent = 0;

        while (1) {
            bytes = SSL_read(ssl, buffer, 1024); /* get request */

            if (bytes == 0) break;

            const unsigned char firstByte = buffer[0];
            const char* content = buffer + 1;

            printf("Message from socket (%d) (packet type: %d) : %s\n", bytes, firstByte, content);

            switch (firstByte) {
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
                    client.file = createFile(client.username, client.password, content, server.rootDir);
                    printf("File opened\n");
                    break;

                case FILE_SIZE:
                    client.fileSize = strtol(content, NULL, 10);
                    printf("File size is %d\n", client.fileSize);
                    break;

                case FILE_CONTENT:
                    if (verifyUser(client.username, client.password)) {
                        if (client.file != NULL) {
                            // Append to file
                            fputs(content, client.file);
                            printf("Data written\n");

                            if (++client.chunkSent >= client.fileSize / CHUNK_SIZE) {
                                printf("File sending is done\n");
                                fclose(client.file);
                            }
                        }

                        printf("User %s sent file data\n", client.username);
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
                                   "405 This is not a http(s) server!", 1024);
                    break;
            }
        }
    }

    sd = SSL_get_fd(ssl);       /* get socket connection */
    SSL_free(ssl);         /* release SSL state */
    close(sd);          /* close connection */

    printf("Connection closed.\n");
}
