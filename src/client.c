// TODO : faire les dossiers nessaires pour la gestion des conffiles cote user : .tucs/ conf/ pubKey/ privKey/
// TODO : SSL - Generation de clefs
// TODO - sub SSL - Gestion des clefs
// TODO : Gestion des fichiers de facon dynamique
// TODO : interface ncurses
// TODO : login / password
// TODO :

#include <stdint.h>
#include <stdio.h>
//#include <string.h>
//#include <stdint.h>
#include <stdlib.h>
#if defined (linux) /* Linux */

    //#include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h> /* close */
    //#include <netdb.h> /* gethostbyname */

#else /* Unknown OS */

    #error not defined for this platform

#endif
// TODO : A mettre dans le fichier de config, dans le cas present, si le chunk size est modifie,
//  il faudra envoyer au serveur la nouvelle config du CHUNK_SIZE
#define CHUNK_SIZE 1024

// TODO : Put this in an other file and include it
// TODO : Faire un packet pour appeller le create user
enum packet_type {
    LOGIN = 0x10,
    USERNAME = 0x11,
    PASSWORD = 0x12,
    PUBKEY = 0x13,

    FILE_ACTION = 0x20,
    CREATE_FILE = 0x21,
    EDIT_FILE = 0x22,
    DELETE_FILE = 0x23,
    READ_FILE = 0x24,
    FILE_SIZE = 0x25,
    FILE_NAME = 0x26,

    FILE_CONTENT = 0x30,
    FILE_CLOSED = 0x40,
};

/**
 * Creates a socket and return its id
 * @param address Server's address
 * @param port Server's port
 * @return Socket's id
 */
int createSocketAndConnect(char *address, int port) {
    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, address, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/Address not supported");
        return -1;
    }

    if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return -1;
    }

    return sock;
}

/**
 * Send file content to given socket
 * @param socket Socket's id
 * @param filename Path to access the file
 * @return
 */
int sendFileToSocket(int sock, const char* filename) {
    FILE* fileptr;
    unsigned char* file_buffer;
    long filelen;

    // Open file in binary mode
    fileptr = fopen(filename, "rb");
    // Jump to end of file
    fseek(fileptr, 0, SEEK_END);
    // Get offset (used for length)
    filelen = ftell(fileptr);
    // Go back to start of file
    rewind(fileptr);

    // Send file information to server
    char* file_info = malloc(sizeof(char) * CHUNK_SIZE);
    // File name
    sprintf(file_info, "%c%s", FILE_NAME, filename);
    send(sock, file_info, CHUNK_SIZE, 0);
    // File size
    sprintf(file_info, "%c%ld", FILE_SIZE, filelen);
    send(sock, file_info, CHUNK_SIZE, 0);
    // Action
    sprintf(file_info, "%c%s", CREATE_FILE, filename);
    send(sock, file_info, CHUNK_SIZE, 0);

    // Allocate memory for the file_buffer
    file_buffer = (unsigned char*) malloc((CHUNK_SIZE - 1) * sizeof(char));

    for (long i = 0; i < filelen; i += (CHUNK_SIZE - 1)) {
        // Move cursor in file
        fseek(fileptr, i, SEEK_SET);
        // Write file data to file_buffer
        fread(file_buffer, 1, (CHUNK_SIZE - 1), fileptr);

        // Create request buffer
        char* file_data = malloc(sizeof(char) * CHUNK_SIZE);
        sprintf(file_data, "%c%s", FILE_CONTENT, file_buffer);

        // Send request
        send(sock, file_data, CHUNK_SIZE, 0);
    }

    // Close file
    fclose(fileptr);
}

/**
 * Function to send credentials to server
 * @param sock
 * @param username
 * @param password
 */
void login(int sock, const char* username, const char* password) {
    char* buffer = malloc(sizeof(char) * CHUNK_SIZE);

    // Send username
    sprintf(buffer, "%c%s", USERNAME, username);
    send(sock, buffer, CHUNK_SIZE, 0);

    // Send password
    sprintf(buffer, "%c%s", PASSWORD, password);
    send(sock, buffer, CHUNK_SIZE, 0);
}
/**
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char const *argv[]) {
    int sock = createSocketAndConnect("151.80.110.124", 8080);

    // Login to server
    // TODO : Verification avec le serveur pour se connecter / creer un compte si inexistant
    login(sock, "quozul", "password");

    // Send message
    sendFileToSocket(sock, argv[1]);

    close(sock);

    return EXIT_SUCCESS;
}
