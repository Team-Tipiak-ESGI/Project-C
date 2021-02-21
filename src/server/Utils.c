#include <openssl/ssl.h>
#include <openssl/md5.h>
#include "MongoConnection.h"
#include "Database.h"
#include "../shared/ChunkSize.h"

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

    int users = MongoConnection__getUser(mongoConnection, username, password);
    printf("Verifying user [%s] [%s] %d...\n", username, password, users);

    return users > 0;
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
// TODO: Fix large chunks not being received correctly
// TODO: Threaded send and receive functions

/**
 * Hash the input to an hexadecimal string
 * @param input
 * @return
 */
char* hexHash(char * input) {
    unsigned char hashedOutput[20];
    char * hashedName = malloc(40);

    // Initialize string to 0
    for (int i = 0; i < 40; ++i) {
        hashedName[i] = 0;
    }

    SHA1((const unsigned char*)input, strlen(input), hashedOutput);

    // Convert SHA to hex string
    char hex[3];
    for (int i = 0; i < 20; i++) {
        sprintf(hex, "%02x", hashedOutput[i]);
        strcat(hashedName, hex);
    }

    return hashedName;
}

/**
 * Return the directory name of the given user
 * @param username
 * @param password
 * @return
 */
char * getUserDir(const char* username, const char* password) {
    char * temp = malloc(1024);
    char * hashInput = malloc(1024);

    // Generate hashed name for the file, should be unique
    strcpy(temp, username);
    strtok(temp, "\0");
    strcpy(hashInput, temp);

    strcpy(temp, password);
    strtok(temp, "\0");
    strcat(hashInput, temp);

    free(temp);

    return hexHash(hashInput);
}

/**
 * Create a folder to store all file's chunks
 * @param username Username of the owner
 * @param password Password of the owner (should be hashed)
 * @param fileName Send file name by the client
 * @param rootDir Directory to store files from server's configuration
 * @return Path of the folder which should contain all the chunks of the file
 */
char* createFile(const char* username, const char* password, const char* fileName, const char* rootDir) {
    char * filePath;
    struct stat st = {0};

    char * hashedNameUser = getUserDir(username, password);
    char * hashedFileName = hexHash((char*) fileName);

    // Generate file path
    filePath = malloc(strlen(rootDir) + strlen(hashedNameUser) + strlen(hashedFileName) + 1);
    strcpy(filePath, rootDir);
    strcat(filePath, hashedNameUser);

    // Create directory if not exists
    if (stat(filePath, &st) == -1) {
        mkdir(filePath, 0700);
    }

    // Add hashed filename to file path
    strcat(filePath, "/");
    strcat(filePath, hashedFileName);

    // Create directory if not exists
    if (stat(filePath, &st) == -1) {
        mkdir(filePath, 0700);
    }

    return filePath;
}

/**
 * Write a file chunk to disk
 * @param originalFilePath
 * @param content
 * @param chunkNumber
 */
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

void writePacket(SSL * ssl, char packet, char * content) {
    char writeBuffer[CHUNK_SIZE];

    if (content != NULL)
        sprintf(writeBuffer, "%c%s", packet, content);
    else
        sprintf(writeBuffer, "%c", packet);

    printf("Writing: [%s]\n", writeBuffer);

    SSL_write(ssl, writeBuffer, CHUNK_SIZE);
}
