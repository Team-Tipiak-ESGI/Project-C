#include <mongoc.h>

MongoConnection* MongoConnection__init();
void MongoConnection__close(MongoConnection* mongoConnection);
void MongoConnection__createUser(MongoConnection* mongoConnection, char* username, char* password);
void MongoConnection__getUser(MongoConnection* mongoConnection, char* username, char* password);