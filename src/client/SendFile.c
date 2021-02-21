#include <stdlib.h>
#include <stdio.h>
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <string.h>

#include "../shared/PacketTypes.h"

// TODO : A mettre dans le fichier de config, dans le cas present, si le chunk size est modifie,
//  il faudra envoyer au serveur la nouvelle config du CHUNK_SIZE
#include "../shared/ChunkSize.h"

/**
 * Send file content to given socket
 * @param socket Socket's id
 * @param filename Path to access the file
 * @return
 */
void sendFileToSocket(SSL *ssl, const char* filename) {
    FILE* filePtr;
    unsigned char* fileBuffer;
    long fileLength;

    char readBuffer[CHUNK_SIZE];

    // Open file in binary mode
    filePtr = fopen(filename, "rb");
    // Jump to end of file
    fseek(filePtr, 0, SEEK_END);
    // Get offset (used for length)
    fileLength = ftell(filePtr);
    // Go back to start of file
    rewind(filePtr);

    // Send file information to server
    char* msg = malloc(sizeof(char) * CHUNK_SIZE);

    // File name
    sprintf(msg, "%c%s", FILE_NAME, filename);
    SSL_write(ssl, msg, CHUNK_SIZE);   /* encrypt & send message */
    // File size
    sprintf(msg, "%c%ld", FILE_SIZE, fileLength);
    SSL_write(ssl, msg, CHUNK_SIZE);   /* encrypt & send message */
    // Action
    sprintf(msg, "%c%s", CREATE_FILE, filename);
    SSL_write(ssl, msg, CHUNK_SIZE);   /* encrypt & send message */

    free(msg);

    // Allocate memory for the fileBuffer
    fileBuffer = (unsigned char*) malloc((CHUNK_SIZE - 1) * sizeof(char));

    for (long i = 0; i < fileLength; i += (CHUNK_SIZE - 1)) {
        // Move cursor in file
        fseek(filePtr, i, SEEK_SET);
        // Write file data to fileBuffer
        fread(fileBuffer, 1, (CHUNK_SIZE - 1), filePtr);

        // Create request buffer
        char* fileData = malloc(sizeof(char) * CHUNK_SIZE);
        sprintf(fileData, "%c%s", FILE_CONTENT, fileBuffer);

        // Send request
        SSL_write(ssl, fileData, CHUNK_SIZE);
        printf("Chunk sent, waiting server confirmation\n");

        do {
            SSL_read(ssl, readBuffer, CHUNK_SIZE);
        } while(readBuffer[0] != CHUNK_RECEIVED && readBuffer[0] != UNAUTHORIZED);

        switch (readBuffer[0]) {
            case UNAUTHORIZED:
                printf("Not authorized!\n");
                break;

            case CHUNK_RECEIVED:
                printf("Chunk received by server!\n");
                break;
        }

        free(fileData);
    }

    free(fileBuffer);

    // Close file
    fclose(filePtr);
}

/**
 * Ask server for file list and print the list
 * @param ssl
 */
char ** listFiles(SSL *ssl) {
    char readBuffer[CHUNK_SIZE];
    char* msg = malloc(sizeof(char) * CHUNK_SIZE);

    sprintf(msg, "%c", LIST_FILES);
    SSL_write(ssl, msg, CHUNK_SIZE);   /* encrypt & send message */

    printf("Chunk sent, waiting server response\n");

    SSL_read(ssl, readBuffer, CHUNK_SIZE);

    char * content = readBuffer + 1;

    // Count files
    int i, count;
    for (i = 0, count = 0; content[i]; i++) count += (content[i] == 1);

    char ** files = malloc(sizeof(char *) * count);

    char * tok = content;

    i = 0;

    // Print the file list
    printf("Files:\n");
    while ((tok = strtok(tok, "\1")) != NULL) {
        printf("  [%s]\n", tok);
        files[i++] = strdup(tok); // Add file to file list
        tok = NULL;
    }

    return files;
}

/**
 * Request a file to the server and saves it to the given location
 * @param ssl
 * @param fileName
 * @param destination
 */
void downloadFile(SSL *ssl, char * fileName, char * destination) {
    char readBuffer[CHUNK_SIZE];
    char* msg = malloc(sizeof(char) * CHUNK_SIZE);

    int fileSize = -1, receivedChunks = 0;
    FILE * file = fopen(destination, "ab+");

    sprintf(msg, "%c%s", READ_FILE, fileName);
    SSL_write(ssl, msg, CHUNK_SIZE);

    printf("Request sent, waiting server response\n");

    do {
        SSL_read(ssl, readBuffer, CHUNK_SIZE);
        char * content = readBuffer + 1;

        printf("Received response: [%s]\n", content);

        switch (readBuffer[0]) {
            case FILE_SIZE:
                fileSize = (unsigned char) *content;
                printf("File size: %d\n", fileSize);
                break;
            case FILE_CONTENT:
                fputs(content, file);
                receivedChunks++;
                printf("Content: %s\n", content);
                break;
        }
    } while (fileSize == -1 || receivedChunks < (double)fileSize / CHUNK_SIZE);

    printf("Done\n");

    fclose(file);
    free(msg);
}

void deleteFile(SSL *ssl, char* fileName) {
    char msg[CHUNK_SIZE] = {0};

    sprintf(msg, "%c%s", DELETE_FILE, fileName);
    SSL_write(ssl, msg, CHUNK_SIZE);

    printf("Request sent\n");
}

/**
 * Function to send credentials to server
 * @param ssl
 * @param username
 * @param password
 */
void login(SSL *ssl, const char* username, const char* password) {
    printf("Logging in with credentials [%s] [%s]...\n", username, password);

    char* buffer = malloc(sizeof(char) * CHUNK_SIZE);

    // Send username
    sprintf(buffer, "%c%s", USERNAME, username);
    SSL_write(ssl, buffer, CHUNK_SIZE);

    // Send password
    sprintf(buffer, "%c%s", PASSWORD, password);
    SSL_write(ssl, buffer, CHUNK_SIZE);
}

/**
 * Create an account with the given credentials
 * @param ssl
 * @param username
 * @param password
 */
int signup(SSL *ssl, const char* username, const char* password) {
    printf("Signing up with credentials [%s] [%s]...\n", username, password);

    char* buffer = malloc(sizeof(char) * CHUNK_SIZE);

    // Send username
    sprintf(buffer, "%c%s", USERNAME, username);
    SSL_write(ssl, buffer, CHUNK_SIZE);

    // Send password
    sprintf(buffer, "%c%s", PASSWORD, password);
    SSL_write(ssl, buffer, CHUNK_SIZE);

    // Ask account creation
    sprintf(buffer, "%c", CREATE_USER);
    SSL_write(ssl, buffer, CHUNK_SIZE);

    // Wait for server's response
    char readBuffer[CHUNK_SIZE];
    SSL_read(ssl, readBuffer, CHUNK_SIZE);

    switch (readBuffer[0]) {
        case USER_CREATED:
            printf("User successfully created!\n");
            return 1;

        case USER_EXISTS:
            printf("A user with the same username already exists!\n");
            return 0;
    }

    return -1;
}