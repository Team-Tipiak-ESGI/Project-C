
void Username   (Client * client, ServerConfiguration * server, SSL * ssl, MongoConnection * mongoConnection, const char * content);
void Password   (Client * client, ServerConfiguration * server, SSL * ssl, MongoConnection * mongoConnection, const char * content);
void CreateUser (Client * client, ServerConfiguration * server, SSL * ssl, MongoConnection * mongoConnection, const char * content);
void CreateFile (Client * client, ServerConfiguration * server, SSL * ssl, MongoConnection * mongoConnection, const char * content);
void FileSize   (Client * client, ServerConfiguration * server, SSL * ssl, MongoConnection * mongoConnection, const char * content);
void FileContent(Client * client, ServerConfiguration * server, SSL * ssl, MongoConnection * mongoConnection, const char * content);
void ReadFile   (Client * client, ServerConfiguration * server, SSL * ssl, MongoConnection * mongoConnection, const char * content);
void DeleteFile (Client * client, ServerConfiguration * server, SSL * ssl, MongoConnection * mongoConnection, const char * content);
void ListFiles  (Client * client, ServerConfiguration * server, SSL * ssl, MongoConnection * mongoConnection, const char * content);
