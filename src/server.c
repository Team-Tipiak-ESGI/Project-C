#include <stdint.h>
#include <stdio.h>
#include <string.h>
//#include <stdint.h>
#include <stdlib.h>

#if defined (linux) /* Linux */

    //#include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    //#include <arpa/inet.h>
    #include <unistd.h> /* close */
    #include <netdb.h> /* gethostbyname */

    #include <openssl/bio.h>
    #include <openssl/ssl.h>
    #include <openssl/err.h>

#else /* Unknown OS */

    #error not defined for this platform

#endif

#define PORT 8080
#define CHUNK_SIZE 1024

#include "PacketTypes.h"

void InitializeSSL() {
    SSL_load_error_strings();
    SSL_library_init();
    OpenSSL_add_all_algorithms();
}

void DestroySSL() {
    ERR_free_strings();
    EVP_cleanup();
}

void ShutdownSSL(SSL *cSSL) {
    SSL_shutdown(cSSL);
    SSL_free(cSSL);
}

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

int createSSLServer() {
    struct sockaddr_in address;

    int server_fd;

    InitializeSSL();
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    if (server_fd < 0) {
        perror("Failed attaching socket");
        exit(EXIT_FAILURE);
    }

    bzero((char *) &address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("Listen error");
        exit(EXIT_FAILURE);
    }

    return server_fd;
}

int main(int argc, char const *argv[]) {
    const int server_fd = createSSLServer();
    fputs("Server listening...\n", stdout);

    int new_socket, address, addrlen, valread;
    char buffer[1024] = {0};

    //while (1) {
        int newsockfd;
        SSL_CTX *sslctx;
        SSL *cSSL;

        // SSL connexion
        int cli_addr;
        unsigned int clilen;

        newsockfd = accept(server_fd, (struct sockaddr *) &cli_addr, &clilen);
        printf("%d\n", newsockfd);
        if (newsockfd < 0) {
            perror("Accept error");
        }

        fputs("New connexion...\n", stdout);

        /*sslctx = SSL_CTX_new(SSLv23_server_method());
        SSL_CTX_set_options(sslctx, SSL_OP_SINGLE_DH_USE);

        int use_cert = SSL_CTX_use_certificate_file(sslctx, "/home/erwan/cert.pem", SSL_FILETYPE_PEM);
        int use_prv = SSL_CTX_use_PrivateKey_file(sslctx, "/key.pem", SSL_FILETYPE_PEM);

        cSSL = SSL_new(sslctx);
        SSL_set_fd(cSSL, newsockfd);
        // Here is the SSL Accept portion. Now all reads and writes must use SSL
        int ssl_err = SSL_accept(cSSL);

        if (ssl_err <= 0) {
            // Error occurred, log and close down ssl
            perror("SSL accept error");
            ShutdownSSL(cSSL);
        }*/

        // Handle user packets

        /*pid_t pid = fork();

        char* username = NULL;
        char* password = NULL;

        // If in fork, then listen for value
        if (pid == 0) {
            while (1) {
                valread = read(ssl_err, buffer, CHUNK_SIZE);
                if (valread == 0) break;

                const char firstByte = buffer[0];
                char* content = buffer + 1;

                printf("Message from socket %d (%d) (packet type %d) : %s\n", ssl_err, valread, firstByte, content);

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

            fputs("Connexion closed...\n", stdout);
        }*/
    //}

    return EXIT_SUCCESS;
}
