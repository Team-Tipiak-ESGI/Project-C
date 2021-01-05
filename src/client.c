// Client side C/C++ program to demonstrate Socket programming
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef WIN32 /* si vous êtes sous Windows */

    #include <winsock.h>

#elif defined (linux) /* si vous êtes sous Linux */

    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h> /* close */
    #include <netdb.h> /* gethostbyname */
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket(s) close(s)
    typedef int SOCKET;
    typedef struct sockaddr_in SOCKADDR_IN;
    typedef struct sockaddr SOCKADDR;
    typedef struct in_addr IN_ADDR;

#else /* sinon vous êtes sur une plateforme non supportée */

    #error not defined for this platform

#endif

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
    unsigned char* buffer;
    long filelen;

    // open file in binary mode
    fileptr = fopen(filename, "rb");
    // jump to end of file
    fseek(fileptr, 0, SEEK_END);
    // get offset (used for length)
    filelen = ftell(fileptr);
    // go back to start of file
    rewind(fileptr);

    // allocate memory for the buffer
    buffer = (unsigned char*) malloc(filelen * sizeof(char));
    // write file data to buffer
    fread(buffer, 1, filelen, fileptr);
    // close file
    fclose(fileptr);

    send(sock, buffer, 1024, 0);
}

int main(int argc, char const *argv[]) {
    int sock = createSocketAndConnect("127.0.0.1", 8080);

    // Send message
    sendFileToSocket(sock, argv[1]);

    close(sock);

    return EXIT_SUCCESS;
}
