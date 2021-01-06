#include <stdint.h>
#include <stdio.h>
#include <string.h>
//#include <stdint.h>
#include <stdlib.h>

#ifdef WIN32 /* Windows */

    //#include <winsock.h>

#elif defined (linux) /* Linux */

    //#include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    //#include <arpa/inet.h>
    #include <unistd.h> /* close */
    #include <netdb.h> /* gethostbyname */

#else /* Unknown OS */

    #error not defined for this platform

#endif

#define PORT 8080
#define CHUNK_SIZE 1024

// TODO: Put this in an other file and include it
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
    FILE_CONTENT = 0x30,
};

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

    const char* valid_username = "quozul\0";
    const char* valid_password = "password\0";

    return (strcmp(username, valid_username) == 0 && strcmp(password, valid_password) == 0);
}

int main(int argc, char const *argv[]) {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *hello = "Hello from server";

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Failed attaching socket");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen error");
        exit(EXIT_FAILURE);
    }

    printf("Server listening...\n");

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *) &address, (socklen_t * ) & addrlen)) < 0) {
            perror("Accept error");
            exit(EXIT_FAILURE);
        }

        pid_t pid = fork();

        char* username = NULL;
        char* password = NULL;

        // If fork, then listen for value
        if (pid == 0) {
            while (1) {
                valread = read(new_socket, buffer, CHUNK_SIZE);
                if (valread == 0) break;

                const char firstByte = buffer[0];
                char* content = buffer + 1;

                printf("Message from socket %d (%d) (packet type %d) : %s\n", new_socket, valread, firstByte, content);

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
                    case FILE_CONTENT:
                        if (verifyUser(username, password)) {
                            printf("User %s sent file data\n", username);
                        } else {
                            printf("User is not logged in!\n");
                        }
                        break;
                    default:
                        break;
                }
            }
        }
    }

    return EXIT_SUCCESS;
}
