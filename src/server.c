// Server side C/C++ program to demonstrate Socket programming
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

#else /* sinon vous êtes sur une plateforme non supportée */

#error not defined for this platform

#endif

#define PORT 8080

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

        // If fork, then listen for value
        if (pid == 0) {
            while (1) {
                valread = read(new_socket, buffer, 1024);
                if (valread == 0) break;

                printf("Message from socket %d (%d) : %s\n", new_socket, valread, buffer);

                send(new_socket, hello, strlen(hello), 0);
            }
        }
    }

    return 0;
}