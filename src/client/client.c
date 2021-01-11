// TODO : faire les dossiers nessaires pour la gestion des conffiles cote user : .tucs/ conf/ pubKey/ privKey/
// TODO : SSL - Generation de clefs
// TODO - sub SSL - Gestion des clefs
// TODO : Gestion des fichiers de facon dynamique
// TODO : interface ncurses
// TODO : login / password
// TODO : A mettre dans le fichier de config, dans le cas present, si le chunk size est modifie,
//  il faudra envoyer au serveur la nouvelle config du CHUNK_SIZE

#include <stdint.h>
#include <stdio.h>
#include <string.h> // bzero
#include <stdlib.h>
#if defined (linux) /* Linux */

    #include <unistd.h> /* close */
    #include <netdb.h> // gethostbyname

    #include <openssl/bio.h>
    #include <openssl/ssl.h>
    #include <openssl/err.h>

#else /* Unknown OS */

    #error not defined for this platform

#endif

#define CHUNK_SIZE 1024

#include "../shared/PacketTypes.h"
#include "connection.h"


int main() {
    SSL_CTX *ctx;
    int server;
    SSL *ssl;
    char buf[1024];
    int bytes;

    SSL_library_init();

    ctx = initCTX();
    server = openConnection("127.0.0.1", 8080);
    ssl = SSL_new(ctx);      /* create new SSL connection state */
    SSL_set_fd(ssl, server);    /* attach the socket descriptor */

    if (SSL_connect(ssl) < 0) {   /* perform the connection */
        ERR_print_errors_fp(stderr);
    } else {
        char *msg = "Hello";

        printf("Connected with %s encryption\n", SSL_get_cipher(ssl));
        SSL_write(ssl, msg, strlen(msg));   /* encrypt & send message */

        bytes = SSL_read(ssl, buf, sizeof(buf)); /* get reply & decrypt */
        buf[bytes] = 0;
        printf("Received: \"%s\"\n", buf);

        SSL_free(ssl);        /* release connection state */
    }

    close(server);         /* close socket */
    SSL_CTX_free(ctx);        /* release context */
    return 0;
}
