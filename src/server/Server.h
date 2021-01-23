#include "MongoConnection.h"

void servlet(SSL *ssl, ServerConfiguration server, MongoConnection* mongoConnection);

unsigned char verifyUser(char* username, char* password, MongoConnection* mongoConnection);
