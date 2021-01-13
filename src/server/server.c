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
void servlet(SSL *ssl, ServerConfiguration serverConfiguration) {
    char buffer[CHUNK_SIZE];
    int sd, bytes;

    if (SSL_accept(ssl) < 0) {     /* do SSL-protocol accept */
        ERR_print_errors_fp(stderr);
    } else {
        char* username;
        char* password;
        FILE* workingOnFile;
        int dataSize = 0;
        int chunkSent = 0;
        char* hashedName = malloc(1024);

        while (1) {
            bytes = SSL_read(ssl, buffer, 1024); /* get request */

            if (bytes == 0) break;

            const unsigned char firstByte = buffer[0];
            const char* content = buffer + 1;

            printf("Message from socket (%d) (packet type: %d) : %s\n", bytes, firstByte, content);

            switch (firstByte) {
                case USERNAME:
                    // Copy username to variable
                    username = malloc(sizeof(char) * strlen(content));
                    strncpy(username, content, strlen(content));
                    break;

                case PASSWORD:
                    // Copy password to variable
                    password = malloc(sizeof(char) * strlen(content));
                    strncpy(password, content, strlen(content));
                    break;

                case CREATE_FILE:
                    workingOnFile = createFile(username, password, content, serverConfiguration.rootDir);
                    printf("File opened\n");
                    break;

                case FILE_SIZE:
                    dataSize = atoi(content);
                    printf("File size is %d\n", dataSize);
                    break;

                case FILE_CONTENT:
                    if (verifyUser(username, password)) {
                        if (workingOnFile != NULL) {
                            // Append to file
                            fputs(content, workingOnFile);
                            printf("Data written\n");

                            if (++chunkSent >= dataSize / CHUNK_SIZE) {
                                printf("File sending is done\n");
                                fclose(workingOnFile);
                            }
                        }

                        printf("User %s sent file data\n", username);
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

        free(username);
        free(password);
        free(hashedName);
    }

    sd = SSL_get_fd(ssl);       /* get socket connection */
    SSL_free(ssl);         /* release SSL state */
    close(sd);          /* close connection */

    printf("Connection closed.\n");
}