#include <stdio.h>

#if defined (linux) /* Linux */

    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h> /* close */
    #include <netdb.h> /* gethostbyname */

    #include <openssl/crypto.h>
    #include <openssl/ssl.h>
    #include <openssl/err.h>

#endif

#define PORT 8080
#define CHUNK_SIZE 1024

#include "../shared/PacketTypes.h"
#include "connection.h"

/**
 * Serve the connection
 * @param ssl
 */
void servlet(SSL *ssl) {
    char buf[1024];
    char reply[1024];
    int sd, bytes;

    if (SSL_accept(ssl) < 0) {     /* do SSL-protocol accept */
        ERR_print_errors_fp(stderr);
    } else {
        while (1) {
            bytes = SSL_read(ssl, buf, sizeof(buf)); /* get request */
            if (bytes > 0) {
                printf("Client msg: \"%s\"\n", buf);
            } else {
                break;
            }
        }

        /*if (bytes > 0) {
            buf[bytes] = 0;
            printf("Client msg: \"%s\"\n", buf);
            sprintf(reply, "Hello OK", buf);   // construct reply
            SSL_write(ssl, reply, strlen(reply)); // send reply
        } else {
            ERR_print_errors_fp(stderr);
        }*/
    }

    sd = SSL_get_fd(ssl);       /* get socket connection */
    SSL_free(ssl);         /* release SSL state */
    close(sd);          /* close connection */
}

int main() {
    SSL_CTX *ctx;
    int server;

    // TODO: Put cert files in config file
    char CertFile[] = "/home/erwan/cert.pem";
    char KeyFile[] = "/home/erwan/key.pem";

    SSL_library_init();

    ctx = initServerCTX();        /* initialize SSL */
    loadCertificates(ctx, CertFile, KeyFile); /* load certs */

    // TODO: Put port in config file
    server = openListener(PORT);    /* create server socket */

    // Main loop
    while (1) {
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        SSL *ssl;

        int client = accept(server, (struct sockaddr *) &addr, &len);  /* accept connection as usual */
        printf("Connection from: %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        ssl = SSL_new(ctx);              /* get new SSL state with context */
        SSL_set_fd(ssl, client);      /* set connection socket to SSL state */

        int pid = fork();

        if (pid == 0) {
            // If in fork
            servlet(ssl);         /* service connection */
        }
    }

    close(server);          /* close server socket */
    SSL_CTX_free(ctx);         /* release context */
}