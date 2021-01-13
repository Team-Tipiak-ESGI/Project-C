void loadCertificates(SSL_CTX *ctx, char *certFile, char *keyFile);

int openConnection(const char *hostname, int port);

SSL_CTX *initCTX(void);
