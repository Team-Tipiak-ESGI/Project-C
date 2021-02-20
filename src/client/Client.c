// TODO : faire les dossiers nessaires pour la gestion des conffiles cote user : .tucs/ conf/ pubKey/ privKey/
// TODO : SSL - Generation de clefs
// TODO - sub SSL - Gestion des clefs
// TODO : Gestion des fichiers de facon dynamique
// TODO : interface ncurses
// TODO : login / password

#include <stdio.h>
#include <string.h> // bzero

#if defined (linux) /* Linux */

    #include <unistd.h> /* close */
    #include <netdb.h> // gethostbyname

    #include <openssl/bio.h>
    #include <openssl/ssl.h>
    #include <openssl/err.h>

#endif

#include "../shared/ChunkSize.h"

#include "../shared/PacketTypes.h"
#include "Connection.h"
#include "SendFile.h"


int main(int argc, char** argv) {
    printf("%d\n", argc);
    
    SSL_CTX *ctx;
    int server;
    SSL *ssl;

    SSL_library_init();

    ctx = initCTX();
    server = openConnection("127.0.0.1", 8080);
    ssl = SSL_new(ctx);      /* create new SSL connection state */
    SSL_set_fd(ssl, server);    /* attach the socket descriptor */

    if (SSL_connect(ssl) < 0) {   /* perform the connection */
        ERR_print_errors_fp(stderr);
    } else {
        printf("Connected with %s encryption\n", SSL_get_cipher(ssl));

        //signup(ssl, "bonjour", "test");

        login(ssl, argv[1], argv[2]);

        sendFileToSocket(ssl, argv[3]);

        char ** files = listFiles(ssl);
        printf("File: %s\n", files[0]);

        readFile(ssl, files[0], "/home/erwan/downloaded");

        SSL_free(ssl);        /* release connection state */
    }

    close(server);         /* close socket */
    SSL_CTX_free(ctx);        /* release context */
    return 0;
}
