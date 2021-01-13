#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <string.h>

#include "../shared/PacketTypes.h"

// TODO : A mettre dans le fichier de config, dans le cas present, si le chunk size est modifie,
//  il faudra envoyer au serveur la nouvelle config du CHUNK_SIZE
#define CHUNK_SIZE 1024

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
        free(fileData);
    }

    free(fileBuffer);

    // Close file
    fclose(filePtr);
}

/**
 * Function to send credentials to server
 * @param ssl
 * @param username
 * @param password
 */
void login(SSL *ssl, const char* username, const char* password) {
    char* buffer = malloc(sizeof(char) * CHUNK_SIZE);

    // Send username
    sprintf(buffer, "%c%s", USERNAME, username);
    SSL_write(ssl, buffer, CHUNK_SIZE);

    // Send password
    sprintf(buffer, "%c%s", PASSWORD, password);
    SSL_write(ssl, buffer, CHUNK_SIZE);
}
