// TODO : faire les dossiers nessaires pour la gestion des conffiles cote user : .tucs/ conf/ pubKey/ privKey/
// TODO : SSL - Generation de clefs
// TODO - sub SSL - Gestion des clefs
// TODO : Gestion des fichiers de facon dynamique
// TODO : interface ncurses
// TODO : login / password

#include <stdio.h>
#include <string.h> // bzero
#include <unistd.h> /* close */
#include <netdb.h> // gethostbyname

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "../shared/ChunkSize.h"

#include "../shared/PacketTypes.h"
#include "Connection.h"
#include "SendFile.h"
#include "window.h"
#include "../shared/Configuration.h"


int main(int argc, char** argv) {
    Item * config = Configuration__loadFromFile("resources/client.conf");

    printf("%d\n", argc);
    
    SSL_CTX *ctx;
    int server;
    SSL *ssl;

    SSL_library_init();

    ctx = initCTX();
    server = openConnection(Item__getByKey(config, "address")->value, atoi(Item__getByKey(config, "port")->value));
    ssl = SSL_new(ctx);      /* create new SSL connection state */
    SSL_set_fd(ssl, server);    /* attach the socket descriptor */

    if (SSL_connect(ssl) < 0) {   /* perform the connection */
        ERR_print_errors_fp(stderr);
    } else {
        printf("Connected with %s encryption\n", SSL_get_cipher(ssl));

        window_login(ssl);

        SSL_free(ssl);        /* release connection state */
    }

    close(server);         /* close socket */
    SSL_CTX_free(ctx);        /* release context */

    return EXIT_SUCCESS;
}
