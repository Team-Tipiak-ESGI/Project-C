int sendFileToSocket(SSL *ssl, const char* filename);

void login(SSL *ssl, const char* username, const char* password);
int signup(SSL *ssl, const char* username, const char* password);

void listFiles(SSL *ssl);