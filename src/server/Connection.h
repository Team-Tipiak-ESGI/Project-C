int openListener(int port);

SSL_CTX *initServerCTX(void);

void loadCertificates(SSL_CTX *ctx, char *certFile, char *keyFile);
